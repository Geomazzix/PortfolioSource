#pragma once
#include <string>

namespace mrpoc
{
#if defined (PLATFORM_SWITCH)
	void strcpy_s(char* dest, rsize_t destsz, const char* src);
#endif

	/// Read an entire text file into a string. Only use for small files.
	///
	/// @filename   Full path to the file
	///
	std::string ReadFileToString(const std::string& filename);

	/// Read an entire text file into a char buffer.
	///
	/// @filename   Full path to the file
	///
	bool ReadFileToBuffer(const std::string& filename, unsigned char*& buffer, int& length);

	/// Save text to a file
	/// @filename   Full path to the file
	/// @text       Text to save
	///
	bool SaveFile(const std::string& filename, const std::string& text);

	/// String hash using sdbm algorithm
	unsigned long long StringHash(const std::string& str);

	/// Replace all occurrences of the search string with the replacement string.
	///
	/// @param subject The string being searched and replaced on, otherwise known as the haystack.
	/// @param search The value being searched for, otherwise known as the needle.
	/// @param replace The replacement value that replaces found search values.
	/// @return a new string with all occurances replaced.
	///
	std::string StringReplace(const std::string& subject,
		const std::string& search,
		const std::string& replace);

	/// Determine whether or not a string starts with the given prefix. Does
	/// not create an internal copy.
	///
	/// @param subject The string being searched in.
	/// @param prefix The string to search for.
	/// @return a boolean indicating if the prefix was found.
	///
	bool StringStartsWith(const std::string& subject,
		const std::string& prefix);

	/// Determine whether or not a string ends with the given suffix. Does
	/// not create an internal copy.
	///
	/// @param subject The string being searched in.
	/// @param prefix The string to search for.
	/// @return a boolean indicating if the suffix was found.
	///
	bool StringEndsWith(const std::string& subject,
		const std::string& suffix);

	/// Makes the first latter capital
	void StringCapitalizeFirst(std::string& str);

	/// Convert codepoint to a UTF8 string
	std::string UnicodeToUTF8(unsigned int codepoint);

	std::string ToString(float f, int precision);

	std::string ToHexString(int number);

	inline void SwitchOnBitFlag(unsigned int& flags, unsigned int bit) { flags |= bit; }

	inline void SwitchOffBitFlag(unsigned int& flags, unsigned int bit) { flags &= (~bit); }

	inline bool CheckBitFlag(unsigned int flags, unsigned int bit) { return (flags & bit) == bit; }

	inline bool CheckBitFlagOverlap(unsigned int flag0, unsigned int flag1) { return (flag0 & flag1) != 0; }

	/// Does hermite spline interpolation of a real value
	template <class T>
	T HermiteInterpolation(T from, T to, float t, float m0, float m1)
	{
		// Put t in [0,1]
		if (t < 0)
			t = 0;
		else if (t > 1)
			t = 1;

		// Interpolate
		float t2 = t * t;		// t squared
		float t3 = t2 * t;		// t cubbed
								//
		T val = (2 * t3 - 3 * t2 + 1) * from; // h00 * p0
		val += (t3 - 2 * t2 + t) * m0;	// h10 * m0
		val += (-2 * t3 + 3 * t2) * to;	// h01 * p1
		val += (t3 - t2) * m1;  // h11 * m1
										//
		return val;
	}

	enum InterpolationType
	{
		INTERPOLATION_LERP,
		INTERPOLATION_EASE_IN,
		INTERPOLATION_EASE_OUT,
		INTERPOLATION_EASE_IN_OUT,
		INTERPOLATION_EASE_MID,
		INTERPOLATION_SPRING
	};


	template <class T>
	T Interpolate(T a, T b, float t, InterpolationType interpolation)
	{
		// Put t in [0,1]
		if (t < 0)
			t = 0;
		else if (t > 1)
			t = 1;

		// Change the interpolation
		switch (interpolation)
		{
		case INTERPOLATION_SPRING:
		case INTERPOLATION_LERP:
			break;

		case INTERPOLATION_EASE_IN:
			t = HermiteInterpolation(0.0f, 1.0f, t, 0.0f, 1.0f);
			break;

		case INTERPOLATION_EASE_OUT:
			t = HermiteInterpolation(0.0f, 1.0f, t, 1.0f, 0.0f);
			break;

		case INTERPOLATION_EASE_IN_OUT:
			t = HermiteInterpolation(0.0f, 1.0f, t, 0.0f, 0.0f);
			break;

		case INTERPOLATION_EASE_MID:
			t = HermiteInterpolation(0.0f, 1.0f, t, 1.0f, 1.0f);
			break;
		}

		// Return a lerp with the
		return Lerp(a, b, t);
	}


	/// Clamps a value in a certain interval
	///
	/// @param val Value that will be clamped
	/// @param from Low bound
	/// @param to High bound
	template <class T>
	T Clamp(T val, T from, T to)
	{
		T t = val;
		if (t < from)
			t = from;
		else if (t > to)
			t = to;
		return t;
	}

	/// Remaps a value from one range to another
	template <class T>
	T RemapRange(T in, T inStart, T inEnd, T outStart, T outEnd)
	{
		T out = in - inStart;		// Puts in [0, inEnd - inStart]
		out /= (inEnd - inStart);	// Puts in [0, 1]
		out *= (outEnd - outStart); // Puts in [0, outRange]
		out += outStart;			// Puts in [outStart, outEnd]
		return out;
	}

	/// Remaps a value from one range to [0,1]
	template <class T>
	T RemapRange(T in, T inStart, T inEnd)
	{
		T out = in - inStart;		// Puts in [0, inEnd - inStart]
		out /= (inEnd - inStart);	// Puts in [0, 1]
		return out;
	}


	inline  float SmootherStep(float edge0, float edge1, float x) {
		// Scale, and clamp x to 0..1 range
		x = Clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
		// Evaluate polynomial
		return x * x * x * (x * (x * 6.f - 15.f) + 10.f);
	}

	long long Now();

}
