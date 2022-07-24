#pragma once

class ISender
{
private:
    /* data */
public:
    ISender(/* args */);
    virtual ~ISender();
    virtual void testdll()=0;
};