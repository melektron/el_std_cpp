/*
ELEKTRON © 2025 - now
Written by melektron
www.elektron.work
23.02.25, 13:49
All rights reserved.

This source code is licensed under the Apache-2.0 license found in the
LICENSE file in the root directory of this source tree. 

Example usage of stack (sort of a test-like code)
*/

#include <el/static_stack.hpp>
#include <cstdio>
#include <cstdint>
#include <vector>


struct MyObject
{
    std::uint32_t num;

    MyObject(int n)
    {
        num = n;
        std::printf("> MyObject Init %p: %d\r\n", this, num);
    }
    MyObject(const MyObject &other)
    {
        num = other.num;
        std::printf("> MyObject Copy %p<-%p: %d\r\n", this, &other, num);
    }
    MyObject(MyObject &&other)
    {
        num = other.num;
        other.num = 0;
        std::printf("> MyObject Move %p<-%p: %d\r\n", this, &other, num);
    }
    ~MyObject()
    {
        std::printf("> MyObject Del  %p\r\n", this);
    }
};

#define STACK_INFO(stack) \
    std::printf("sizeof(" #stack ")=%d\r\n", sizeof((stack))); \
    std::printf(#stack ".size()=%d\r\n", (int)(stack).size()); \
    std::printf(#stack ".overflowed()=%d\r\n", (int)(stack).overflowed())


int main()
{
    std::printf("\r\n== empty construct\r\n");
    el::static_stack<MyObject, 5> stack1;
    STACK_INFO(stack1);

    std::printf("\r\n== initializer list constructor (with overflow)\r\n");
    el::static_stack<MyObject, 5> stack2 = {
        {5}, {6}, {7}, {8}, {9}, {10}
    };
    STACK_INFO(stack2);
    std::printf("Clear overflow\r\n");
    stack2.clear_overflow();
    STACK_INFO(stack2);

    std::printf("\r\n== range constructor (with overflow)\r\n");
    std::vector<MyObject> input_data = {
        {20}, {56}, {89}, {99}, {110}, {129}
    };
    el::static_stack<MyObject, 5> stack3(input_data.begin(), input_data.end());
    STACK_INFO(stack3);
    std::printf("Clear overflow\r\n");
    stack3.clear_overflow();
    STACK_INFO(stack3);

    std::printf("\r\n== copy constructor (with overflow)\r\n");
    el::static_stack<MyObject, 4> stack4(stack3);
    STACK_INFO(stack3);
    STACK_INFO(stack4);
    std::printf("Clear overflow\r\n");
    stack4.clear_overflow();
    STACK_INFO(stack4);

    std::printf("\r\n== move constructor (with overflow)\r\n");
    el::static_stack<MyObject, 4> stack5(std::move(stack3));
    STACK_INFO(stack3);
    STACK_INFO(stack5);
    std::printf("Clear overflow\r\n");
    stack5.clear_overflow();
    STACK_INFO(stack5);


    std::printf("\r\n== push/pop while not empty\r\n");
    while (!stack4.empty())
    {
        stack1.push(std::move(*stack4.top()));
        std::printf("stack4.pop()=%d\r\n", (int)stack4.pop());
        STACK_INFO(stack1);
        STACK_INFO(stack4);
    }

    std::printf("\r\n== range iterate and popping\r\n");
    bool pop_result = false;
    do {
        std::printf("items:\r\n");
        for (const auto &v : stack2)
            std::printf(" - %d\n\r", v.num);
        
        pop_result = stack2.pop();
        std::printf("stack2.pop()=%d\r\n", (int)pop_result);
    } while (pop_result);

    return 0;
}
