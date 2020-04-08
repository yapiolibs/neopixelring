#pragma once

//--------------------------------------------------------------------------------------------------

template <uint16_t MAX> struct CappedNumber
{
    CappedNumber(const CappedNumber &other)
    {
        value = other.value;
        trim();
    }
    CappedNumber(uint16_t val)
    {
        value = val;
        trim();
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator++()
    {
        operator+=(1);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator--()
    {
        operator-=(1);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber operator++(int)
    {
        CappedNumber temp{ value };
        operator+=(1);
        return temp;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber operator--(int)
    {
        CappedNumber temp{ value };
        operator-=(1);
        return temp;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber operator+(uint16_t sum) { return { value + sum }; }

    //----------------------------------------------------------------------------------------------

    CappedNumber operator-(uint16_t sum)
    {
        CappedNumber temp{ value };
        temp -= sum; // handle potential underflow with care
        return temp;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber operator+(const CappedNumber &other)
    {
        this->operator+(other.value);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber operator-(const CappedNumber &other)
    {
        this->operator-(other.value);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator=(const CappedNumber &other)
    {
        this->operator=(other.value);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator=(uint16_t val)
    {
        value = val;
        trim();
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator-=(const CappedNumber &other)
    {
        this->operator-=(other.value);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator+=(const CappedNumber &other)
    {
        this->operator+=(other.value);
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator-=(const int16_t val)
    {
        if(val < 0) // increment
            value += -val;
        else // decrement
        {
            if(val > value) // on underflow
                value = MAX - (val - value);
            else
                value -= val;
        }
        trim();
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    CappedNumber &operator+=(int16_t val)
    {
        if(val > 0) // increment
            value += val;
        else // decrement
        {
            if(val + value < 0) // on underflow
                value = MAX + (val + value);
            else
                value -= -val;
        }

        trim();
        return *this;
    }

    //----------------------------------------------------------------------------------------------

    bool operator==(const CappedNumber &other) { return operator==(other.value); }

    bool operator!=(const CappedNumber &other) { return !operator==(other.value); }

    bool operator==(uint16_t val) { return value == val; }

    bool operator!=(uint16_t val) { return !operator==(val); }

    operator uint16_t() { return value; }

private:
    inline void trim() { value %= MAX; }

    uint16_t value{ 0 };
};