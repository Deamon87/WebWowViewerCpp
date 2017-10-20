//From https://gist.github.com/nlguillemot/3112646
#ifndef HASHEDSTRING_HPP_INCLUDED
#define HASHEDSTRING_HPP_INCLUDED

/////////////
// LINKING //
/////////////

//////////////
// INCLUDES //
//////////////

/////////////
// DEFINES //
/////////////

/*
	=> Pre-processor constant hashed strings only are created if those flags..:
		- C/C++ > Optimization > Full Optimization
		- C/C++ > Code Generation > Enable String Pooling
	are set to true... So, if you wanna use this, remember to change the project properties.
*/

////////////////////////////////////////////////////////////////////////////////
// Class name: FHashedString
////////////////////////////////////////////////////////////////////////////////
class HashedString
{
public:

    // Used to return the hash component
    class Hasher
    {
        friend HashedString;

    public:
        size_t operator()(const HashedString & _hashedString) const
        {
            return _hashedString.m_Hash;
        }
    };

    // Compare 2 hashed strings
    class Equal
    {
        friend HashedString;

    public:
        bool operator() (HashedString const& t1, HashedString const& t2) const
        {
            return (t1.m_Hash == t2.m_Hash);
        }
    };

private:

    // Just a constant wrapper
    struct ConstCharWrapper
    {
        inline ConstCharWrapper(const char* str);
        const char* str;
    };

public:

    // A wrapper so we can generate any number of functions using the pre-processor (const strings)
    template <size_t N>
    constexpr HashedString(const char(&str)[N]);
    explicit constexpr HashedString(ConstCharWrapper str);

    // Return the original string
#ifdef _DEBUG
    const char* String()
    {
        return m_String;
    }
#endif
    // Return the hash
    const size_t Hash() const
    {
        return m_Hash;
    }

public:

    // The hash object (pre-calculated if we use the full optimization flag)
    size_t m_Hash;

    // The original string
#ifdef _DEBUG
    const char* m_String;
#endif
};

/////////////////////////////////
// IGNORE ANY ERROR FROM THERE // => Those macros are fine
/////////////////////////////////

#define OFFSET 2166136261u
#define PRIME 16777619u

#define ME_JOIN(x,y)				x##y

#define ME_HASHED_STRING_1            OFFSET
#define ME_HASHED_STRING_2            ((ME_HASHED_STRING_1 ^ str[0]) * PRIME)
#define ME_HASHED_STRING_3            ((ME_HASHED_STRING_2 ^ str[1]) * PRIME)
#define ME_HASHED_STRING_4            ((ME_HASHED_STRING_3 ^ str[2]) * PRIME)
#define ME_HASHED_STRING_5            ((ME_HASHED_STRING_4 ^ str[3]) * PRIME)
#define ME_HASHED_STRING_6            ((ME_HASHED_STRING_5 ^ str[4]) * PRIME)
#define ME_HASHED_STRING_7            ((ME_HASHED_STRING_6 ^ str[5]) * PRIME)
#define ME_HASHED_STRING_8            ((ME_HASHED_STRING_7 ^ str[6]) * PRIME)
#define ME_HASHED_STRING_9            ((ME_HASHED_STRING_8 ^ str[7]) * PRIME)
#define ME_HASHED_STRING_10            ((ME_HASHED_STRING_9 ^ str[8]) * PRIME)
#define ME_HASHED_STRING_11            ((ME_HASHED_STRING_10 ^ str[9]) * PRIME)
#define ME_HASHED_STRING_12            ((ME_HASHED_STRING_11 ^ str[10]) * PRIME)
#define ME_HASHED_STRING_13            ((ME_HASHED_STRING_12 ^ str[11]) * PRIME)
#define ME_HASHED_STRING_14            ((ME_HASHED_STRING_13 ^ str[12]) * PRIME)
#define ME_HASHED_STRING_15            ((ME_HASHED_STRING_14 ^ str[13]) * PRIME)
#define ME_HASHED_STRING_16            ((ME_HASHED_STRING_15 ^ str[14]) * PRIME)
#define ME_HASHED_STRING_17            ((ME_HASHED_STRING_16 ^ str[15]) * PRIME)
#define ME_HASHED_STRING_18            ((ME_HASHED_STRING_17 ^ str[16]) * PRIME)
#define ME_HASHED_STRING_19            ((ME_HASHED_STRING_18 ^ str[17]) * PRIME)
#define ME_HASHED_STRING_20            ((ME_HASHED_STRING_19 ^ str[18]) * PRIME)
#define ME_HASHED_STRING_21            ((ME_HASHED_STRING_20 ^ str[19]) * PRIME)
#define ME_HASHED_STRING_22            ((ME_HASHED_STRING_21 ^ str[20]) * PRIME)
#define ME_HASHED_STRING_23            ((ME_HASHED_STRING_22 ^ str[21]) * PRIME)
#define ME_HASHED_STRING_24            ((ME_HASHED_STRING_23 ^ str[22]) * PRIME)
#define ME_HASHED_STRING_25            ((ME_HASHED_STRING_24 ^ str[23]) * PRIME)
// etc.

#define ME_HASHED_STRING_SPECIALIZATION(n)                                    \
  template <>                                                                \
  constexpr HashedString::HashedString(const char (&str)[n])                    \
    : m_Hash(ME_JOIN(ME_HASHED_STRING_, n))                                \
  {}

ME_HASHED_STRING_SPECIALIZATION(1)
ME_HASHED_STRING_SPECIALIZATION(2)
ME_HASHED_STRING_SPECIALIZATION(3)
ME_HASHED_STRING_SPECIALIZATION(4)
ME_HASHED_STRING_SPECIALIZATION(5)
ME_HASHED_STRING_SPECIALIZATION(6)
ME_HASHED_STRING_SPECIALIZATION(7)
ME_HASHED_STRING_SPECIALIZATION(8)
ME_HASHED_STRING_SPECIALIZATION(9)
ME_HASHED_STRING_SPECIALIZATION(10)
ME_HASHED_STRING_SPECIALIZATION(11)
ME_HASHED_STRING_SPECIALIZATION(12)
ME_HASHED_STRING_SPECIALIZATION(13)
ME_HASHED_STRING_SPECIALIZATION(14)
ME_HASHED_STRING_SPECIALIZATION(15)
ME_HASHED_STRING_SPECIALIZATION(16)
ME_HASHED_STRING_SPECIALIZATION(17)
ME_HASHED_STRING_SPECIALIZATION(18)
ME_HASHED_STRING_SPECIALIZATION(19)
ME_HASHED_STRING_SPECIALIZATION(20)
ME_HASHED_STRING_SPECIALIZATION(21)
ME_HASHED_STRING_SPECIALIZATION(22)
ME_HASHED_STRING_SPECIALIZATION(23)
ME_HASHED_STRING_SPECIALIZATION(24)
ME_HASHED_STRING_SPECIALIZATION(25)

static size_t CalculateFNV(const char* str)
{
    unsigned int hash = OFFSET;
    while (*str != 0)
    {
        hash ^= *str++;
        hash *= PRIME;
    }

    return hash;
}


#endif // HASHEDSTRING_HPP_INCLUDED