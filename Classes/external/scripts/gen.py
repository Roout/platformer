import json
import re
import sys
import argparse
import uuid
import time
from datetime import datetime
from typing import Any, Dict, List, Set, Tuple

def snake_to_lower_camel(variable: str):
  assert len(variable) > 0
  assert variable[0].islower()
  return re.sub('(_[a-z])', lambda m: m.group(1)[1].upper(), variable)

def snake_to_upper_camel(variable: str):
  assert len(variable) > 0
  variable = variable[0].upper() + variable[1:]
  return re.sub('(_[a-z])', lambda m: m.group(1)[1].upper(), variable)

def upper_camel_to_snake(variable: str):
  assert len(variable) > 0
  assert variable[0].isupper()
  prefix = variable[0].lower()
  suffix = re.sub('([A-Z])', lambda m: '_' + m.group(1).lower(), variable[1:])
  return prefix + suffix

def lower_camel_to_snake(variable: str):
  assert len(variable) > 0
  assert variable[0].islower()
  return re.sub('([A-Z])', lambda m: '_' + m.group(1).lower(), variable)

class CodeGenerator:
  methods_by_type: Dict[str, str] = None
  cpptype_by_python_type: Dict[str, str] = None
  
  def __init__(self):
      self.scopes: List[str] = []
      self.structs: List[List[Tuple]] = []
      self.used: Set[str] = set()
      self.footer: str = ''

      # this is all supported types and array:
      # std::array<one_of_supported_type, ...>
      if CodeGenerator.methods_by_type == None:
        # initialize static variables
        CodeGenerator.methods_by_type = {
          'bool': 'GetBool()', 
          'float': 'GetFloat()', 
          'int': 'GetInt()', 
          'std::string': 'GetString()' }

        assert CodeGenerator.cpptype_by_python_type == None
        CodeGenerator.cpptype_by_python_type = {
          'bool': 'bool',
          'str': 'std::string',
          'float': 'float',
          'list': 'std::array',
          'int': 'int' }

  @staticmethod
  def cpp_numeric(typename: str, value):
    s = str(value)
    if s.find('.') >= 0:
      s += 'f'
    elif typename == 'float':
      s += '.f'
    return s

  # `json_field` - json field in snake case
  # return cpp variable name in lower camel
  @staticmethod
  def cpp_var_name(json_field: str):
    assert lower_camel_to_snake(snake_to_lower_camel(json_field)) == json_field, 'conversation is not bidirectional'
    return snake_to_lower_camel(json_field)
    
  # `variable` - cpp variable in lower camel
  # return json field in snake case
  @staticmethod
  def json_var_name(variable: str):
    assert snake_to_lower_camel(lower_camel_to_snake(variable)) == variable, 'conversation is not bidirectional'
    return lower_camel_to_snake(variable)
  
  # `json_field` - json field in snake case
  # return cpp class name in upper camel
  @staticmethod
  def cpp_class_name(json_field: str):
    assert upper_camel_to_snake(snake_to_upper_camel(json_field)) == json_field, 'conversation is not bidirectional'
    return snake_to_upper_camel(json_field)

  @staticmethod
  def cpp_var_type(python_typename: str):
    if python_typename not in CodeGenerator.cpptype_by_python_type:
      raise ValueError('Unsupported python to cpp typename conversation')
    return CodeGenerator.cpptype_by_python_type[python_typename]

  # `object_instance` - array owner
  def array_parser_func(self, indent: str, array_type: str, array_name: str, object_instance: str):
    assert array_type.find('std::array') != -1, 'Expect `std::array<T, size>` string'
    array_parser = ''

    matched = re.search(r'< *([^,]+) *, *([0-9]+) *>', array_type)
    assert matched != None
    innertype = matched.groups(2)[0]
    innersize = matched.groups(2)[1]

    if innertype not in CodeGenerator.methods_by_type:
      if innertype not in self.used:
        raise TypeError('This custom(?) type is not supported')
      method = None
    else:
      method = CodeGenerator.methods_by_type[innertype]
    array_parser += '{}{{\n'.format(indent)
    array_parser += '{}  const auto& values = json["{}"].GetArray();\n'.format(indent, array_name)
    array_parser += '{}  for (size_t i = 0; i < {}; i++) {{\n'.format(indent, innersize)
    if method is None:
      # parse custom user-defined type
      # use `void FromJson(const rapidjson::Value& json, T& obj)`
      array_parser += '{}    FromJson(values[i], {}.{}[i]);\n'.format(indent, object_instance, array_name)
    else:
      # parse one of basic type (key in `CodeGenerator.methods_by_type` dictionary)
      array_parser += '{}    {}.{}[i] = values[i].{};\n'.format(indent, object_instance, array_name, method)
    array_parser += '{}  }}\n'.format(indent)
    array_parser += '{}}}\n'.format(indent)

    return array_parser

  # `class_name` is a cpp struct name
  # `struct` is a cpp struct definition with all it's data members as dictionary
  def dump_json_parser_func(self, class_name: str, struct: list[tuple]):
    """generate and dump `FromJson(const rapidjson::Value& json, T& obj)` 
    to string buffer - `self.footer`"""

    # create object/variable name from class name
    obj_instance = class_name[0].lower() + class_name[1:]
    self.footer += 'inline void FromJson(const rapidjson::Value& json, {}& {}) {{\n'.format(
      class_name, obj_instance)
    
    for typename, variable in struct:
      if typename in self.used:
        # it's an object of custom type
        self.footer += '  FromJson(json["{}"], {}.{});\n'.format(
          self.json_var_name(variable),
          obj_instance,
          variable)
      elif typename.find('std::array') != -1:
        self.footer += self.array_parser_func('  ', typename, variable, obj_instance)
      else:
        self.footer += '  {}.{} = json["{}"].{};\n'.format(
          obj_instance, 
          variable,
          self.json_var_name(variable),
          CodeGenerator.methods_by_type[typename])

    self.footer += '}\n\n'

  # dump class definition to ostream
  @staticmethod
  def dump_class(class_name: str, struct: list[tuple], ostream = sys.stdout):
    print('struct {} {{'.format(class_name), file = ostream)
    for typename, varaible in struct:
      print('  {} {};'.format(typename, varaible), file = ostream)
    print('};\n', file = ostream)
  
  # generate a class name as concatenation of class scopes
  # it returns the name that is not used yet
  def choose_class_name(self):
    assert len(self.scopes) > 0
    class_name = str()
    for scope in reversed(self.scopes):
      class_name = scope + class_name
      if class_name not in self.used:
        self.used.add(class_name)
        return class_name
    return None

  def generate(self, json_value: Any, ostream = sys.stdout):
    if not isinstance(json_value, dict):
      return

    for key in json_value:
      if isinstance(json_value[key], dict):
        # add(open) class declaration i.e., create dict
        self.structs.append([])
        self.scopes.append(CodeGenerator.cpp_class_name(key))
        self.generate(json_value[key], ostream)
        typename = self.choose_class_name()
        if typename == None:
          raise RuntimeError('all class names are already reserved')
        # remove(close) class declaration
        self.scopes.pop()
        struct = self.structs.pop()
        if len(self.structs) > 0:
          self.structs[-1] += [(typename, CodeGenerator.cpp_var_name(key))]
        CodeGenerator.dump_class(typename, struct, ostream)
        self.dump_json_parser_func(typename, struct)
      
      elif isinstance(json_value[key], list): # key = menuitem # json_value = popup 
        array_size = len(json_value[key])
        assert array_size > 0, "wrong JSON sample: empty list as input"
        
        if isinstance(json_value[key][0], dict):
          # this is array of custom objects
          # I expect they have same set of keys while values can differ 
          # TODO: throw exception if the expectations failed

          # treat array of objects as struct with array data member:
          # `key` becomes a struct whereas array becomes a data member
          self.structs.append([])
          self.scopes.append(CodeGenerator.cpp_class_name(key))
          # generate struct for one of those fields (take 0th)
          self.generate(json_value[key][0], ostream)
          array_value_type = self.choose_class_name()
          if array_value_type == None:
            raise RuntimeError('all class names are already reserved')

          self.scopes.pop()
          struct = self.structs.pop()
          assert len(self.structs) > 0, 'Array must have an owner (outer struct)'

          # add array as data member of outer-struct
          self.structs[-1] += [
            ('std::array<{}, {}>'.format(array_value_type, array_size), 
            CodeGenerator.cpp_var_name(key))]
          CodeGenerator.dump_class(array_value_type, struct, ostream)
          self.dump_json_parser_func(array_value_type, struct)

        else:
          array_value_type = type(json_value[key][0]).__name__
          array_value_type = CodeGenerator.cpp_var_type(array_value_type)
          # declare array variable
          self.structs[-1] += [
            ('std::array<{}, {}>'.format(array_value_type, array_size), 
            CodeGenerator.cpp_var_name(key))]
      else:
        value_type = type(json_value[key]).__name__
        value_type = CodeGenerator.cpp_var_type(value_type)
        # declare float/int/bool/std::string variable 
        self.structs[-1] += [(value_type, CodeGenerator.cpp_var_name(key))]

def main():
  begin = time.time()

  parser = argparse.ArgumentParser(description = 'Generate a c++ file from the json schema')
  parser.add_argument('-i', '--input',  help = 'path to json schema')
  parser.add_argument('-o', '--output', help = 'c++ file which will be generated from json schema')
  args = parser.parse_args()
  input = args.input
  output = args.output
  UNIQUE_ID = str(uuid.uuid4())

  template = (
    '// This file is auto generated by script\n' + 
    '// Date: {}')
  include_guard = '__JSON_AUTOGENERATED_CLASSES_{}__'.format(UNIQUE_ID.replace('-', '_'))
  includes = (
    '#include <string>\n' + 
    '#include <array>\n\n' + 
    '#include "rapidjson/document.h"\n' + 
    '#include "rapidjson/writer.h"\n' + 
    '#include "rapidjson/stringbuffer.h"\n')

  data = {}
  with open(input, 'r') as istream:
    data = json.load(istream)
  with open(output, 'w') as ostream:
    print(template.format(datetime.now().strftime("%d/%m/%Y %H:%M:%S")), file = ostream)
    print('#ifndef {}'.format(include_guard), file = ostream)
    print('#define {}\n'.format(include_guard), file = ostream)
    print(includes, file = ostream)
    print('namespace json_autogenerated_classes {\n', file = ostream)
    code_generator = CodeGenerator()
    code_generator.generate(data, ostream)
    print('} // namespace json_autogenerated_classes\n', file = ostream)
    print('namespace json_autogenerated_classes {\n', file = ostream)
    print(code_generator.footer, file = ostream)
    print('} // namespace json_autogenerated_classes\n', file = ostream)
    print('#endif // {}'.format(include_guard), file = ostream)

  print('Generate "{}".\nElapsed time: {}s'.format(output, time.time() - begin))

if __name__ == "__main__":
  main()