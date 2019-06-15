#pragma once

#include <random>
#include <thread>
#include <chrono>

#include <CoreFoundation/CoreFoundation.h>
#include <ApplicationServices/ApplicationServices.h>

struct vec2
{
    float x, y;
    
    vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) {}
    vec2(const vec2&) = default;
    vec2& operator=(const vec2&) = default;
    vec2(vec2&&) = default;
    vec2& operator=(vec2&&) = default;
};

class random_helper
{
    
public:
    
    template<typename T>
    static T random_real(T min, T max)
    {
        std::uniform_real_distribution<T> dist(min, max);
        auto& mt = random_helper::engine();
        return dist(mt);
    }
    
    template<typename T>
    static T random_int(T min, T max)
    {
        std::uniform_int_distribution<T> dist(min, max);
        auto& mt = random_helper::engine();
        return dist(mt);
    }
    
private:
    
    static std::mt19937& engine()
    {
        static std::random_device seed_gen;
        static std::mt19937 engine(seed_gen());
        return engine;
    }
    
};

template<typename T> inline T random(T min, T max)
{
    return random_helper::random_int<T>(min, max);
}

template<> inline float random(float min, float max)
{
    return random_helper::random_real(min, max);
}

template<> inline long double random(long double min, long double max)
{
    return random_helper::random_real(min, max);
}

template<> inline double random(double min, double max)
{
    return random_helper::random_real(min, max);
}

class mouse
{
    
public:
    
    enum class button_type { e_left, e_right };
    
    static void move(float x, float y, float offset_min = 0.0f, float offset_max = 0.0f)
    {
        constexpr auto spd = 2.0;
        auto to = vec2(x + random(offset_min, offset_max), y + random(offset_min, offset_max));
        wind(get_position(), to, 9.0, 3.0, 10.0 / spd, 15.0 / spd, 10.0 * spd, 10.0 * spd);
    }
    
    static void click(button_type b)
    {
        mouse_button(b, true);  // press
        mouse_button(b, false); // release
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    static vec2 get_position()
    {
        CGEventRef event = CGEventCreate(NULL);
        CGPoint cursor = CGEventGetLocation(event);
        CFRelease(event);
        return vec2(cursor.x, cursor.y);
    }
    
private:
    
    static void set_position(const vec2& p)
    {
        CGPoint location = CGPointMake(p.x, p.y);
        mouse_event(kCGMouseButtonLeft, kCGEventMouseMoved, location);
    }
    
    static void mouse_event(CGMouseButton button, CGEventType type, CGPoint location)
    {
        CGEventRef event = CGEventCreateMouseEvent(NULL, type, location, button);
        CGEventSetType(event, type);
        CGEventPost(kCGHIDEventTap, event);
        CFRelease(event);
    }
    
    static void wind(vec2 start, vec2 end, double gravity,
                     double wind, double min_wait, double max_wait,
                     double max_step, double target_area)
    {
        const double sqrt2 = sqrt(2.0);
        const double sqrt3 = sqrt(3.0);
        const double sqrt5 = sqrt(5.0);
        
        double xs = double(start.x);
        double ys = double(start.y);
        double xe = double(end.x);
        double ye = double(end.y);
        
        double wind_x = 0.0;
        double wind_y = 0.0;
        double velo_x = 0.0;
        double velo_y = 0.0;
        
        while (hypot(xs - xe, ys - ye) > 1)
        {
            double dist = hypot(xs - xe, ys - ye);
            wind = std::min(wind, dist);
            if (dist >= target_area)
            {
                wind_x = (wind_x / sqrt3) + (random(0.0, ceil(wind) * 2 + 1) - wind) / sqrt5;
                wind_y = (wind_y / sqrt3) + (random(0.0, ceil(wind) * 2 + 1) - wind) / sqrt5;
            }
            else
            {
                wind_x = wind_x / sqrt2;
                wind_y = wind_y / sqrt2;
                if (max_step < 3)
                {
                    max_step = random(0, 3) + 3.0;
                }
                else
                {
                    max_step = max_step / sqrt5;
                }
            }
            velo_x = velo_x + wind_x;
            velo_y = velo_y + wind_y;
            velo_x = velo_x + gravity * (xe - xs) / dist;
            velo_y = velo_y + gravity * (ye - ys) / dist;
            
            if (hypot(velo_x, velo_y) > max_step)
            {
                double random_dist = max_step / 2.0 + (random(0.0, ceil(max_step) / 2));
                double velo_mag = sqrt(velo_x * velo_x + velo_y * velo_y);
                velo_x = (velo_x / velo_mag) * random_dist;
                velo_y = (velo_y / velo_mag) * random_dist;
            }
            
            double last_x = ceil(xs);
            double last_y = ceil(ys);
            xs = xs + velo_x;
            ys = ys + velo_y;
            if (last_x != ceil(xs) || last_y != ceil(ys))
            {
                set_position(vec2(ceil(xs), ceil(ys)));
            }
            
            double step = hypot(xs - last_x, ys - last_y);
            double s = ceil((max_wait - min_wait) * (step / max_step) + min_wait);
            std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<long long>(s)));
        }
        
        if ((ceil(xe) != ceil(xs)) || (ceil(ye) != ceil(ys)))
        {
            set_position(end);
        }
    }
    
    static void mouse_button(button_type b, bool is_press)
    {
        CGEventType type;
        CGMouseButton button;
        
        switch (b)
        {
            case button_type::e_left:
            {
                type = (is_press ? kCGEventLeftMouseDown : kCGEventLeftMouseUp);
                button = kCGMouseButtonLeft;
                break;
            }
            case button_type::e_right:
            {
                type = (is_press ? kCGEventRightMouseDown : kCGEventRightMouseUp);
                button = kCGMouseButtonRight;
                break;
            }
            default: return ;
        };
        
        CGEventRef event = CGEventCreate(NULL);
        CGPoint location = CGEventGetLocation(event); // current mouse location
        CFRelease(event);
        
        mouse_event(button, type, location);
    }

};
