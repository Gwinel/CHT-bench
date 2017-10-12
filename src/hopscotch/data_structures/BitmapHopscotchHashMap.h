#ifndef __BITMAP_HOPSCOTCH_HASHMAP__
#define __BITMAP_HOPSCOTCH_HASHMAP__

////////////////////////////////////////////////////////////////////////////////
// ConcurrentHopscotchHashMap Class
//
////////////////////////////////////////////////////////////////////////////////
//TERMS OF USAGE
//------------------------------------------------------------------------------
//
//	Permission to use, copy, modify and distribute this software and
//	its documentation for any purpose is hereby granted without fee,
//	provided that due acknowledgments to the authors are provided and
//	this permission notice appears in all copies of the software.
//	The software is provided "as is". There is no warranty of any kind.
//
//Authors:
//	Maurice Herlihy
//	Brown University
//	and
//	Nir Shavit
//	Tel-Aviv University
//	and
//	Moran Tzafrir
//	Tel-Aviv University
//
//	Date: July 15, 2008.  
//
////////////////////////////////////////////////////////////////////////////////
// Programmer : Moran Tzafrir (MoranTza@gmail.com)
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <limits.h>
#include "math.h"
#include "memory.h"
#include "../framework/cpp_framework.h"

////////////////////////////////////////////////////////////////////////////////
//INNER CLASSES
////////////////////////////////////////////////////////////////////////////////
class HASH_INT {
public:
	//you must define the following fields and properties
	static const unsigned int _EMPTY_HASH;
	static const unsigned int _BUSY_HASH;
	static const int _EMPTY_KEY;
	static const int _EMPTY_DATA;

	inline static unsigned int Calc(int key) {
		key ^= (key << 15) ^ 0xcd7dcd7d;
		key ^= (key >> 10);
		key ^= (key <<  3);
		key ^= (key >>  6);
		key ^= (key <<  2) + (key << 14);
		key ^= (key >> 16);
		return key;
	}

	inline static bool IsEqual(int left_key, int right_key) {
		return left_key == right_key;
	}

	inline static void relocate_key_reference(int volatile& left, const int volatile& right) {
		left = right;
	}

	inline static void relocate_data_reference(int volatile& left, const int volatile& right) {
		left = right;
	}
};
////////////////////////////////////////////////////////////////////////////////
// CLASS: ConcurrentHopscotchHashMap
////////////////////////////////////////////////////////////////////////////////
template <typename	_tKey,
		    typename	_tData,
			 typename	_tHash,
			 typename	_tLock,
			 typename	_tMemory>
class BitmapHopscotchHashMap {
private:

	// Inner Classes ............................................................
	struct Bucket {
		_u32   volatile	_hopInfo;
		_u32	 volatile	_hash;
		_tKey  volatile	_key;
		_tData volatile	_data;
		void init() {
			_hopInfo	= 0U;
			_hash		= _tHash::_EMPTY_HASH;
			_key		= _tHash::_EMPTY_KEY;
			_data		= _tHash::_EMPTY_DATA;
		}
	};

	struct Segment {
		_u32 volatile	_timestamp;
		_tLock			_lock;

		void init() {
			_timestamp = 0;
			_lock.init();
		}
	};

	// Fields ...................................................................
	_u32 volatile		_segmentShift;
	_u32 volatile		_segmentMask;
	_u32 volatile		_bucketMask;
	Segment*	volatile	_segments;
	Bucket* volatile	_table;

	// Constants ................................................................
	static const _u32 _HOP_RANGE		= 32;
	static const _u32	_INSERT_RANGE	= 4*1024;
	static const _u32 _RESIZE_FACTOR = 2;

	// Small Utilities ..........................................................
	void find_closer_free_backet(const Segment* const start_seg, Bucket** free_backet, _u32* free_distance) {
		Bucket* move_backet( *free_backet - (_HOP_RANGE - 1) );
		for (int move_free_dist(_HOP_RANGE - 1); move_free_dist > 0; --move_free_dist) {
			_u32 start_hop_info(move_backet->_hopInfo);
			int move_new_free_distance(-1);
			_u32 mask(1);
			for (int i(0); i < move_free_dist; ++i, mask <<= 1) {
				if (mask & start_hop_info) {
					move_new_free_distance = i;
					break;
				}
			}
			if (-1 != move_new_free_distance) {
				Segment*	const move_segment(&(_segments[((move_backet - _table) >> _segmentShift) & _segmentMask]));
				
				if(start_seg != move_segment)
					move_segment->_lock.lock();	

				if (start_hop_info == move_backet->_hopInfo) {
					Bucket* new_free_backet(move_backet + move_new_free_distance);
					(*free_backet)->_data  = new_free_backet->_data;
					(*free_backet)->_key   = new_free_backet->_key;
					(*free_backet)->_hash  = new_free_backet->_hash;

					++(move_segment->_timestamp);
					_tMemory::write_barrier();

					move_backet->_hopInfo |= (1U << move_free_dist);
					move_backet->_hopInfo &= ~(1U << move_new_free_distance);

					*free_backet = new_free_backet;
					*free_distance -= move_free_dist;

					if(start_seg != move_segment)
						move_segment->_lock.unlock();	
					return;
				}
				if(start_seg != move_segment)
					move_segment->_lock.unlock();	
			}
			++move_backet;
		}
		*free_backet = 0; 
		*free_distance = 0;
	}

	
public:// Ctors ................................................................

	BitmapHopscotchHashMap(_u32 inCapacity, _u32 concurrencyLevel) 
	:	_segmentMask  ( NearestPowerOfTwo(concurrencyLevel) - 1),
		_segmentShift ( CalcDivideShift(NearestPowerOfTwo(concurrencyLevel/(NearestPowerOfTwo(concurrencyLevel)))-1) )
	{
		//ADJUST INPUT ............................
		const _u32 adjInitCap = NearestPowerOfTwo(inCapacity);
		const _u32 adjConcurrencyLevel = NearestPowerOfTwo(concurrencyLevel);
		const _u32 num_buckets( adjInitCap + _INSERT_RANGE + 1);
		_bucketMask = adjInitCap - 1;
		//_segmentShift = first_msb_bit_indx(_bucketMask) - first_msb_bit_indx(_SEGMENTS_MASK);

		//ALLOCATE THE SEGMENTS ...................
		_segments = (Segment*) _tMemory::byte_aligned_malloc( (_segmentMask + 1) * sizeof(Segment) );
		_table = (Bucket*) _tMemory::byte_aligned_malloc( num_buckets * sizeof(Bucket) );

		Segment* curr_seg = _segments;
		for (_u32 iSeg = 0; iSeg <= _segmentMask; ++iSeg, ++curr_seg) {
			curr_seg->init();
		}

		Bucket* curr_bucket = _table;
		for (_u32 iElm=0; iElm < num_buckets; ++iElm, ++curr_bucket) {
			curr_bucket->init();
		}
	}

	~BitmapHopscotchHashMap() {
		_tMemory::byte_aligned_free(_table);
		_tMemory::byte_aligned_free(_segments);
	}

	// Query Operations .........................................................
	inline_ bool containsKey(const _tKey key) {
		//CALCULATE HASH ..........................
		const unsigned int hash( _tHash::Calc(key) );

		//CHECK IF ALREADY CONTAIN ................
		const	Segment&	segment(_segments[(hash >> _segmentShift) & _segmentMask]);
		register const Bucket* const elmAry( &(_table[hash & _bucketMask]) );
		register _u32 hopInfo( elmAry->_hopInfo );

		if(0U ==hopInfo)
			return false;
		else if(1U == hopInfo ) {
			if(hash == elmAry->_hash && _tHash::IsEqual(key, elmAry->_key))
				return true;
			else return false;
		} /*else if(2U == hopInfo) {
			const Bucket* currElm( elmAry);  ++currElm;
			if(hash == currElm->_hash && _tHash::IsEqual(key, currElm->_key))
				return true;
			else return false;
		}*/

		const	_u32 startTimestamp( segment._timestamp );
		while(0U != hopInfo) {
			register const int i( first_lsb_bit_indx(hopInfo) );
			register const Bucket* currElm( elmAry + i);
			if(hash == currElm->_hash && _tHash::IsEqual(key, currElm->_key))
				return true;
			hopInfo &= ~(1U << i);
		} 

		//-----------------------------------------
		if( segment._timestamp == startTimestamp)
			return false;

		//-----------------------------------------
		register const	Bucket* currBucket( &(_table[hash & _bucketMask]) );
		for(int i(0); i<_HOP_RANGE; ++i, ++currBucket) {
			if(hash == currBucket->_hash && _tHash::IsEqual(key, currBucket->_key))
				return true;
		}
		return false;
	}

	//modification Operations ...................................................
	inline_ _tData putIfAbsent(const _tKey key,  const _tData data) {
		//CALCULATE HASH ..........................
		const unsigned int hash( _tHash::Calc(key) );

		//LOCK KEY HASH ENTERY ....................
		Segment&	segment(_segments[(hash >> _segmentShift) & _segmentMask]);
		segment._lock.lock();
		Bucket* const startBucket( &(_table[hash & _bucketMask]) );

		//CHECK IF ALREADY CONTAIN ................
		register _u32 hopInfo( startBucket->_hopInfo );
		while(0 != hopInfo) {
			register const int i( first_lsb_bit_indx(hopInfo) );
			const Bucket* currElm( startBucket + i);
			if(hash == currElm->_hash && _tHash::IsEqual(key, currElm->_key)) {
				register const _tData rc(currElm->_data);
				segment._lock.unlock();
				return rc;
			}
			hopInfo &= ~(1U << i);
		}

		//LOOK FOR FREE BUCKET ....................
		register Bucket* free_bucket( startBucket );
		register _u32 free_distance(0);
		for(; free_distance < _INSERT_RANGE; ++free_distance, ++free_bucket) {
			if( (_tHash::_EMPTY_HASH == free_bucket->_hash) &&	(_tHash::_EMPTY_HASH == _tMemory::compare_and_set(&(free_bucket->_hash), _tHash::_EMPTY_HASH, _tHash::_BUSY_HASH)) )
				break;
		}

		//PLACE THE NEW KEY .......................
		if (free_distance < _INSERT_RANGE) {
			do {
				if (free_distance < _HOP_RANGE) {
					free_bucket->_data   = data;
					free_bucket->_key		= key;
					free_bucket->_hash   = hash;
					startBucket->_hopInfo |= (1U << free_distance);
					segment._lock.unlock();
					return _tHash::_EMPTY_DATA;
				}
				find_closer_free_backet(&segment, &free_bucket, &free_distance);
			} while (0 != free_bucket);
		}

		//NEED TO RESIZE ..........................
		fprintf(stderr, "ERROR - RESIZE is not implemented - size %u\n", size());
		exit(1);
		return _tHash::_EMPTY_DATA;
	}

	inline_ _tData remove( const _tKey key ) {
		//CALCULATE HASH ..........................
		const unsigned int hash( _tHash::Calc(key) );

		//CHECK IF ALREADY CONTAIN ................
		Segment&	segment( _segments[(hash >> _segmentShift) & _segmentMask] );
		segment._lock.lock();
		Bucket* const	startBucket( &(_table[hash & _bucketMask]) );
		register _u32 hopInfo( startBucket->_hopInfo );

		if(0U ==hopInfo) {
			segment._lock.unlock();
			return false;
		} else if(1U == hopInfo) {
			if(hash == startBucket->_hash && _tHash::IsEqual(key, startBucket->_key)) {
				startBucket->_hopInfo &= ~1U;
				startBucket->_hash = _tHash::_EMPTY_HASH;
				startBucket->_key = _tHash::_EMPTY_KEY;
				register const _tData rc(startBucket->_data);
				startBucket->_data = _tHash::_EMPTY_DATA;
				segment._lock.unlock();
				return rc;
			} else {
				segment._lock.unlock();
				return false;
			}
		}

		do {
			register const int i( first_lsb_bit_indx(hopInfo) );
			Bucket* currElm( startBucket + i);
			if(hash == currElm->_hash && _tHash::IsEqual(key, currElm->_key)) {
				register _u32 mask(1); mask <<= i;
				startBucket->_hopInfo &= ~(mask);
				currElm->_hash = _tHash::_EMPTY_HASH;
				currElm->_key = _tHash::_EMPTY_KEY;
				register const _tData rc(currElm->_data);
				currElm->_data = _tHash::_EMPTY_DATA;
				segment._lock.unlock();
				return rc;
			}

			hopInfo &= ~(1U << i);
		} while(0 != hopInfo);

		//UNLOCK & RETURN WITH NULL DATA ..........
		segment._lock.unlock();
		return _tHash::_EMPTY_DATA;
	}

	//status Operations .........................................................
	_u32 size()	{
		_u32 counter = 0;
		const _u32 num_elm( _bucketMask + _INSERT_RANGE );
		for(_u32 iElm=0; iElm < num_elm; ++iElm) {
			if( _tHash::_EMPTY_HASH != _table[iElm]._hash ) {
				++counter;
			}
		}
		return counter;
	}   

	//public final boolean isEmpty();

private:
	// Private Static Utilities .................................................
	static _u32 NearestPowerOfTwo(const _u32 value)	{
		_u32 rc( 1 );
		while (rc < value) {
			rc <<= 1;
		}
		return rc;
	}

	static unsigned int CalcDivideShift(const unsigned int _value) {
		unsigned int numShift( 0 );
		unsigned int curr( 1 );
		while (curr < _value) {
			curr <<= 1;
			++numShift;
		}
		return numShift;
	}

};//ConcurrentHopscotchHashMapBitmap

#endif
