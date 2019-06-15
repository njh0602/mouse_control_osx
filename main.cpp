#include "mouse.hpp"

int main(int argc, char* argv[]) 
{
    mouse::move(100, 100);
    mouse::click(mouse::button_type::e_right);
}
