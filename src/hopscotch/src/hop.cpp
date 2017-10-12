
#include "hopscotch.hpp"

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
