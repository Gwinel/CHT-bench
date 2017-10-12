
////////////////////////////////////////////////////////////////////////////////
// Concurrent Hopscotch Hash Map
//
////////////////////////////////////////////////////////////////////////////////
//TERMS OF USAGE
//----------------------------------------------------------------------
//
//  Permission to use, copy, modify and distribute this software and
//  its documentation for any purpose is hereby granted without fee,
//  provided that due acknowledgements to the authors are provided and
//  this permission notice appears in all copies of the software.
//  The software is provided "as is". There is no warranty of any kind.
//
//Programmers:
//  Hila Goel
//  Tel-Aviv University
//  and
//  Maya Gershovitz
//  Tel-Aviv University
//  
//
//  Date: January, 2015.
////////////////////////////////////////////////////////////////////////////////
//
// This code was developed as part of "Workshop on Multicore Algorithms" 
// at Tel-Aviv university, under the guidance of Prof. Nir Shavit and Moshe Sulamy.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef HOPSCOTCH_HPP_
#define HOPSCOTCH_HPP_

using namespace std;

#include<iostream>
#include<pthread.h>
#include<immintrin.h>
#include<malloc.h>
#include<atomic>

// This is one of eight implementations we created for the workshop.
// In this implementation we used a transactional memory method.
// We used the Haswell's built in transactional memory functions - 
// xbegin() and xend(). 
// When executing an action that changes the data, 
// we initially try to run it as an atomic action,
// if this attempt fails, we run the action using the fine-grained locking method.

class Hopscotch {
private:
	static const int HOP_RANGE = 32;
	static const int ADD_RANGE = 256;
	static const int MAX_SEGMENTS = 1048576; // Including neighbourhood for last hash location
	int* BUSY;

	/*Bucket is the table object.
	Each bucket contains a key and data pairing (as in a usual hashmap),
	and an "hop_info" variable, containing the information 
	regarding all the keys that were initially mapped to the same bucket.*/
	struct Bucket {
	
		volatile atomic_uint _hop_info;
		volatile atomic_intptr_t _key;
		volatile atomic_intptr_t _data;
		/*We used lock_mutex, lock_cv and a volatile variable
		in order to implement a reentrant lock.
		The locking and unlocking methods are a part of the Bucket class*/
		volatile bool _lock;
		pthread_mutex_t lock_mutex;
		pthread_cond_t lock_cv;

		/*The Bucket class constructor*/
		Bucket() {
			_hop_info.store(0);
			_lock = false;
			_key.store(-1);
			_data.store(-1);
			pthread_mutex_init(&lock_mutex, NULL);
			pthread_cond_init(&lock_cv, NULL);
		}

		/*The Bucket class destructor*/
		~Bucket() {
			pthread_mutex_destroy(&lock_mutex);
			pthread_cond_destroy(&lock_cv);
		}

		void lock() {
			pthread_mutex_lock(&lock_mutex);

			while(1) {
				if(_lock == false) {
					_lock = true;
					pthread_mutex_unlock(&lock_mutex);
					break;
				}
				pthread_cond_wait(&lock_cv, &lock_mutex);
			}
		}

		void unlock() {
			pthread_mutex_lock(&lock_mutex);
			_lock = false;
			pthread_cond_signal(&lock_cv);
			pthread_mutex_unlock(&lock_mutex);
		}

	};

	/*A pointer to the table*/
	Bucket* segments_arys;

public:
	Hopscotch();
	~Hopscotch();

	/*inline void trial()
	This is a method used for debugging purposes*/
	inline void trial() {
		Bucket* temp;
		int count = 0, hopCount = 0;
		for(int i = 0; i < MAX_SEGMENTS+256; i++) {
			temp = segments_arys + i;
			if((temp->_key).load() != -1) {
				count++;
			}
			if(temp->_hop_info.load() != 0) {
				hopCount++;
			}
		}
		cout << "Items in Hash = " << count << endl;
		cout << "--------------------" << endl;
	}

	bool add(int *key, int *data);
	int remove(int* key);
	bool contains(int *key);
	void find_closer_bucket(Bucket**, int*, int &);

//private:
	/*inline bool contains(int *key)
	Key - the key we'd like to search for in the table
	Returns true if the table contains the key, and false otherwise*/
	/*
	inline bool contains(int *key) {
		unsigned int hash = ((*key)&(MAX_SEGMENTS-1));
		Bucket* start_bucket = segments_arys+hash;
		unsigned int hop_info = start_bucket->_hop_info.load();
		unsigned int mask = 1;
		
		for(int i = 0; i < HOP_RANGE; ++i, mask <<= 1) {
			if(mask & hop_info) {
				Bucket* check_bucket = start_bucket+i;
				if(*key == (check_bucket->_key).load()) {
					start_bucket->unlock();
					return true;
				}
			}
		}
		return false;
	}
	*/
};

#endif /* HOPSCOTCH_HPP_ */

/*Constructor for the Hopscotch class*/
Hopscotch::Hopscotch() {
	segments_arys = new Bucket[MAX_SEGMENTS+256];
	BUSY = (int *)malloc(sizeof(int));
	*BUSY = -1;
}

/*Destructor for the Hopscotch class*/
Hopscotch::~Hopscotch() {
	delete [] segments_arys;
	free(BUSY);
}

bool Hopscotch::contains(int *key) {
		unsigned int hash = ((*key)&(MAX_SEGMENTS-1));
		Bucket* start_bucket = segments_arys+hash;
		unsigned int hop_info = start_bucket->_hop_info.load();
		unsigned int mask = 1;
		
		for(int i = 0; i < HOP_RANGE; ++i, mask <<= 1) {
			if(mask & hop_info) {
				Bucket* check_bucket = start_bucket+i;
				if(*key == (check_bucket->_key).load()) {
					start_bucket->unlock();
					//cerr<<endl;
					return true;
				}
			}
		}
		return false;
	}
/*int* remove(int *key)
Key - the key we'd like to remove from the table
Returns the data paired with key, if the table contained the key,
and -1 otherwise*/
int Hopscotch::remove(int *key) {
	unsigned int hash = ((*key)&(MAX_SEGMENTS-1));
	Bucket* start_bucket = segments_arys+hash;

	unsigned int hop_info = start_bucket->_hop_info.load();
	unsigned int mask = 1;
	for(int i = 0; i < HOP_RANGE; ++i, mask <<= 1) {
		if(mask & hop_info) {
			Bucket* check_bucket = start_bucket+i;
			
				if (_xbegin() == 0xFFFFFFFF) {
					if (-1 == (check_bucket->_key).load()) {
						_xend();
						return -1;
					}
					if(*key == (check_bucket->_key).load()) {
						int rc = check_bucket->_data.load();
						check_bucket->_key.store(-1);
						check_bucket->_data.store(-1);
						start_bucket->_hop_info.fetch_and(~(1<<i));
						_xend();
						//cout<<"rc1 = "<<rc<<endl;
						return rc;
					}
					else {
						_xend();
						return -1;
					}
				} else {
					start_bucket->lock();
					if (-1 == (check_bucket->_key).load()) {
						start_bucket->unlock();
						return -1;
					}
					if(*key == (check_bucket->_key).load()) {
						int rc = check_bucket->_data.load();
						check_bucket->_key.store(-1);
						check_bucket->_data.store(-1);
						start_bucket->_hop_info.fetch_and(~(1<<i));
						start_bucket->unlock();
						//cout<<"rc2= "<<rc<<endl;
						return rc;
					}
					else {
						start_bucket->unlock();
						return -1;
					}
				}
			}
	}
	return -1;
}
	

/*void find_closer_bucket(Bucket** free_bucket, int* free_distance, int& val)
free_bucket - an empty bucket in the table
free_distance - the function return a value via this var
val - the function return a value via this var

Returns in "free_distance" the distance between start_bucket and the newly freed bucket
Returns in val 0, if it was able to free a bucket in the neighbourhood of start_bucket,
otherwise, val remains unchanged*/
void Hopscotch::find_closer_bucket(Bucket** free_bucket, int* free_distance, int& val) {
	Bucket* move_bucket = *free_bucket - (HOP_RANGE-1);
	int max_free_dist = (HOP_RANGE - 1) > *free_distance ? *free_distance : (HOP_RANGE - 1);
	
	for(int free_dist = max_free_dist; free_dist > 0; --free_dist) {
		unsigned int start_hop_info = move_bucket->_hop_info.load();
		int move_free_distance = -1;
		unsigned int mask = 1;
		for(int i = 0; i < free_dist; ++i, mask <<= 1) {
			if(mask & start_hop_info) {
				move_free_distance = i;
				break;
			}
		}
		/*When a suitable bucket is found, it's content is moved to the old free_bucket*/
		if(-1 != move_free_distance) {
			if(_xbegin() == 0xFFFFFFFF) {
				if(start_hop_info == move_bucket->_hop_info.load()) {
					Bucket* new_free_bucket = move_bucket + move_free_distance;
					/*Updates move_bucket's hop_info, to indicate the newly inserted bucket*/
					move_bucket->_hop_info.fetch_or(1 << free_dist);
					(*free_bucket)->_data.store(new_free_bucket->_data.load());
					(*free_bucket)->_key.store(new_free_bucket->_key.load());
					new_free_bucket->_key.store(*BUSY);
					new_free_bucket->_data.store(*BUSY);
					/*Updates move_bucket's hop_info, to indicate the deleted bucket*/
					move_bucket->_hop_info.fetch_and(~(1<<move_free_distance));
					*free_bucket = new_free_bucket;
					*free_distance = *free_distance - free_dist + move_free_distance;
					_xend();
					return;
				}
			} else {
				move_bucket->lock();
				if(start_hop_info == move_bucket->_hop_info.load()) {
					Bucket* new_free_bucket = move_bucket + move_free_distance;
					move_bucket->_hop_info.fetch_or(1 << free_dist);
					(*free_bucket)->_data.store(new_free_bucket->_data.load());
					(*free_bucket)->_key.store(new_free_bucket->_key.load());
					new_free_bucket->_key.store(*BUSY);
					new_free_bucket->_data.store(*BUSY);
					move_bucket->_hop_info.fetch_and(~(1<<move_free_distance));
					*free_bucket = new_free_bucket;
					*free_distance = *free_distance - free_dist + move_free_distance;
					move_bucket->unlock();
					return;
				}
				move_bucket->unlock();
			}

		}
		++move_bucket;
	}
	(*free_bucket)->_key.store(-1);
	val = 0;
	*free_distance = 0;
	return;
}

/*bool add(int *key,int *data)
Key, Data - the key and data pair we'd like to add to the table.
Returns true if the operation was successful, and false otherwise*/
bool Hopscotch::add(int *key, int *data) {
	int val = 1;
	unsigned int hash = ((*key)&(MAX_SEGMENTS-1));
	Bucket* start_bucket = segments_arys+hash;
	Bucket* free_bucket = start_bucket;
	int free_distance = 0;
	for(; free_distance < ADD_RANGE; ++free_distance) {
		if(-1 == free_bucket->_key.load()) {
			free_bucket->_key.store(*BUSY);
			break;
		}
		++free_bucket;
	}

	if(free_distance < ADD_RANGE) {
		do{
			if(free_distance < HOP_RANGE) {
				/*Inserts the new bucket to the free space*/
				if (_xbegin() == 0xFFFFFFFF) {
					if(contains(key) == false) {
						start_bucket->_hop_info.fetch_or(1<<free_distance);
						free_bucket->_data.exchange(*data);
						free_bucket->_key.exchange(*key);
						_xend();
						return true;
					}
				} else {
					start_bucket->lock();
					if(contains(key) == false) {
						start_bucket->_hop_info.fetch_or(1<<free_distance);
						free_bucket->_data.exchange(*data);
						free_bucket->_key.exchange(*key);
						start_bucket->unlock();
						return true;
					}
					start_bucket->unlock();
				}
				return false;
			} else {
				/*In case a free space was not found in the neighbourhood of start_bucket,
				Clears such a space*/
				find_closer_bucket(&free_bucket, &free_distance, val);
			}
		}while(0 != val);
	}
	//cout << "Called Resize" << endl;
	return false;
}
