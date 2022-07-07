/***************************************************************************//**
 * @file bitset.h
 * @author Dorian Weber
 * @brief Contains the implementation of a bitset.
 ******************************************************************************/

#ifndef BITSET_H_INCLUDED
#define BITSET_H_INCLUDED

#include <string.h>
#include <limits.h>

/**@brief Defines the integer base of the bitset.
 * 
 * The wider this type is, the more algorithms benefit from increased
 * parallelism, but also more space may be wasted, since only complete units of
 * this type may be reserved.
 */
typedef unsigned int bitset_t;

/* *** public interface ***************************************************** */

/**@brief Initializes the bitset for initial usage.
 * @note Simply clears all the bits.
 */
static inline int
bitsetInit(bitset_t* set, unsigned bits, unsigned val);

/**@brief Sets a bit.
 * @param set  target bit set
 * @param bit  index of the bit to set
 */
static inline void
bitsetSet(bitset_t* set, unsigned bit);

/**@brief Clears a bit.
 * @param set  target bit set
 * @param bit  index of the bit to set
 */
static inline void
bitsetClear(bitset_t* set, unsigned bit);

/**@brief Flips a bit.
 * @param set  target bitset
 * @param bit  index of the bit to flip
 */
static inline void
bitsetFlip(bitset_t* set, unsigned bit);

/**@brief Sets or clears a bit conditionally according to a variable flag.
 * @param set   the target bitset
 * @param bit   the bit to modify
 * @param flag  the boolean flag
 */
static inline void
bitsetMod(bitset_t* set, unsigned bit, unsigned flag);

/**@brief Returns the current value of the passed bit.
 * @param set  target bit set
 * @param bit  index of the bit to retrieve
 * @return 1 if the bit is set,\n
 *         0 otherwise
 */
static inline unsigned int
bitsetGet(const bitset_t* set, unsigned bit);

/**@brief Calculates the number of bitset units needed to store the given
 * number of bits.
 * @note This function is implemented as a macro in order to allow static stack
 * allocation in functions with a constant number of bits.
 * 
 * @param bits  number of bits to store
 * @return number of bitset units
 */
#define bitsetSize(bits) \
	(((bits)+CHAR_BIT*sizeof(bitset_t)-1)/(CHAR_BIT*sizeof(bitset_t)))

/**@brief Calculates the number of bytes needed to store a given number of bits.
 * @note This function is implemented as a macro in order to allow static stack
 * allocation in functions with a constant number of bits.
 * 
 * @param bits  number of bits to store
 * @return number of bytes, aligned to bitset_t word boundaries
 */
#define bitsetByteSize(bits) \
	((((bits)+CHAR_BIT-1)/CHAR_BIT + sizeof(bitset_t)-1) &~ (sizeof(bitset_t)-1))

/* *** inline implementation ************************************************ */

static inline int
bitsetInit(bitset_t* set, unsigned bits, unsigned val)
{
	memset(set, (val ? 0xFF : 0), bitsetByteSize(bits));
	return 0;
}

static inline void
bitsetSet(bitset_t* set, unsigned bit)
{
	enum { bitsPerWord = sizeof(bitset_t)*CHAR_BIT };
	set[bit / bitsPerWord] |= ((bitset_t)(1) << (bit % bitsPerWord));
}

static inline void
bitsetClear(bitset_t* set, unsigned bit)
{
	enum { bitsPerWord = sizeof(bitset_t)*CHAR_BIT };
	set[bit / bitsPerWord] &= ~((bitset_t)(1) << (bit % bitsPerWord));
}

static inline void
bitsetFlip(bitset_t* set, unsigned bit)
{
	enum { bitsPerWord = sizeof(bitset_t)*CHAR_BIT };
	set[bit / bitsPerWord] ^= ((bitset_t)(1) << (bit % bitsPerWord));
}

static inline void
bitsetMod(bitset_t* set, unsigned bit, unsigned f)
{
	enum { bitsPerWord = sizeof(bitset_t)*CHAR_BIT };
	const bitset_t m = ((bitset_t)(1) << (bit % bitsPerWord));
	bit /= bitsPerWord;
	set[bit] = (set[bit] & ~m) | (-!!f & m);
}

static inline unsigned int
bitsetGet(const bitset_t* set, unsigned bit)
{
	enum { bitsPerWord = sizeof(bitset_t)*CHAR_BIT };
	return (set[bit / bitsPerWord] >> (bit % bitsPerWord)) & 1;
}

#endif	/* BITSET_H_INCLUDED */
