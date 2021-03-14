#ifndef EASY_TIMER_HPP__
#define EASY_TIMER_HPP__

class EasyTimer {
public:

    bool IsFinished() const noexcept {
        return m_timer <= 0.f;
    }
    
    void Start(float timer) noexcept {
        m_timer = timer;
    }

    void Update(float dt) noexcept {
        if(m_timer > 0.f) {
            m_timer -= dt;
        }
    }

private:

    float m_timer { 0.f };
};

#endif // EASY_TIMER_HPP__