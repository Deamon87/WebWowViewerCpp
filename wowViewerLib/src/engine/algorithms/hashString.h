//From https://gist.github.com/nlguillemot/3112646
#ifndef HASHEDSTRING_HPP_INCLUDED
#define HASHEDSTRING_HPP_INCLUDED

// if HASHEDSTRING_USE_CONSTEXPR is defined, a recursive constexpr hashing algorithm will be used.
// otherwise, an iterative runtime hashing algorithm will be used.
#include <cstdint>

#define HASHEDSTRING_USE_CONSTEXPR

// do not touch this block of preprocessor.
#ifdef HASHEDSTRING_USE_CONSTEXPR
#define HASHEDSTRING_CONSTEXPR_IMPL constexpr
#else
#define HASHEDSTRING_CONSTEXPR_IMPL
#endif

// encapsulates hashed strings
// will try to remember the original string in debug builds.
// in release builds, will not store the string at all.
// strings are hashed at compile time whenever possible using constexpr, if the HASHEDSTRING_USE_CONSTEXPR flag is defined.
    class HashedString
    {
    public:
        typedef uint32_t HashType;

        friend HASHEDSTRING_CONSTEXPR_IMPL HashType _hashString(const char* str);
        friend HASHEDSTRING_CONSTEXPR_IMPL HashType _hashStringRecursive(HashType hash, const char* str);

        // Hashes the passed in string and stores the hash
        // in debug builds, the original string pointer will also be stored to be accessed with getDebugString()
        inline explicit HASHEDSTRING_CONSTEXPR_IMPL HashedString(const char* str);

        template <size_t N>
         inline HASHEDSTRING_CONSTEXPR_IMPL HashedString(const char (&str)[N]);


        // in debug builds, will return the originally hashed string.
        // in debug builds, causes undefined behaviour if the string originally used to construct the hash no longer exists.
        // in release builds, will return an empty string. Should not call this function in release builds.
        inline const char* getReverseHash() const;

        // compares hash by equality
        inline bool operator==(const HashedString& other) const;

        // compares hash in terms of order to allow efficient use with std::less<T> as a Compare function in std::maps
        inline bool operator<(const HashedString& other) const;
        // add more comparison operators as they are needed. Always compare with the hash, not the reverse hash.

        operator HashType () const { return hash; }
    private:
        // reverse hash only exists in debug build
#ifdef _DEBUG
        const char* debugReverseHash;
#endif
        HashType hash;
    };

// performs a compile time recursive string hash using the djb2 algorithm explained here: http://www.cse.yorku.ca/~oz/hash.html
// To hash a string, do not call these functions. Instead, construct a HashedString with the string passed as an argument to the constructor.
    HASHEDSTRING_CONSTEXPR_IMPL HashedString::HashType _hashStringRecursive(HashedString::HashType hash, const char* str)
    {
        return ( !*str ? hash :
                 _hashStringRecursive(((hash << 5) + hash) + *str, str + 1));
    }

// performs a compile time string hash
    HASHEDSTRING_CONSTEXPR_IMPL HashedString::HashType _hashString(const char* str)
    {
#ifdef HASHEDSTRING_USE_CONSTEXPR
        return ( !str ? 0 :
                 _hashStringRecursive(5381, str));
#else
        HashType hash = 5381;
	int x;

	while ( (x = *str++) )
	{
		hash += ((hash << 5) + hash) + x;
	}

	return hash;
#endif
    }

    HASHEDSTRING_CONSTEXPR_IMPL HashedString::HashedString(const char* str):
#ifdef _DEBUG
            debugReverseHash(str),
#endif
            hash(_hashString(str))
    {
    }

    const char* HashedString::getReverseHash() const
    {
#ifdef _DEBUG
        return debugReverseHash;
#else
        return "";
#endif
    }

    bool HashedString::operator==(const HashedString& other) const
    {
        return hash == other.hash;
    }

    bool HashedString::operator<(const HashedString& other) const
    {
        return hash < other.hash;
    }

template<size_t N>
constexpr HashedString::HashedString(const char (&str)[N]) : HashedString(str) {


}


#endif // HASHEDSTRING_HPP_INCLUDED