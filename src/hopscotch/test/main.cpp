////////////////////////////////////////////////////////////////////////////////
// Multi threaded benchmark example
//
////////////////////////////////////////////////////////////////////////////////
//TERMS OF USAGE
//----------------------------------------------------------------------
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
//      and
//	Nir Shavit
//	Tel-Aviv University
//      and
//	Moran Tzafrir
//	Tel-Aviv University
//
// Date: Dec 2, 2008.
//
////////////////////////////////////////////////////////////////////////////////
// Programmer : Moran Tzafrir (MoranTza@gmail.com)
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//INCLUDE DIRECTIVES
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include "../framework/cpp_framework.h"

#include "Configuration.h"
#include "../data_structures/BitmapHopscotchHashMap.h"
#include "../data_structures/ChainedHashMap.h"
#include "../data_structures/HopscotchHashMap.h"

using namespace CMDR;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//CONSTS
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//FORWARD DECLARETIONS
////////////////////////////////////////////////////////////////////////////////
void PrepareRandomNumbers();
void FillTable(int table_size);
unsigned int RunBenchmark();

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

	inline_ static unsigned int Calc(int key) {
		key ^= (key << 15) ^ 0xcd7dcd7d;
		key ^= (key >> 10);
		key ^= (key <<  3);
		key ^= (key >>  6);
		key ^= (key <<  2) + (key << 14);
		key ^= (key >> 16);
		return key;
	}

	inline_ static bool IsEqual(int left_key, int right_key) {
		return left_key == right_key;
	}

	inline_ static void relocate_key_reference(int volatile& left, const int volatile& right) {
		left = right;
	}

	inline_ static void relocate_data_reference(int volatile& left, const int volatile& right) {
		left = right;
	}
};

const unsigned int HASH_INT::_EMPTY_HASH = 0;
const unsigned int HASH_INT::_BUSY_HASH  = 1;
const int HASH_INT::_EMPTY_KEY  = 0;
const int HASH_INT::_EMPTY_DATA = 0;

//Interface to the data-structure we decided to benchmark
class TestDs {
public:
	virtual bool containsKey(int key)=0;
	virtual int get(int key)=0;
	virtual int put(int key, int value)=0;
	virtual int remove(int key)=0;

	virtual int size()=0;
	virtual const char* name()=0;
	virtual void print()=0;
	virtual void shutdown()=0;
};

class TestBitmapHopscotch : public TestDs {
private:
	BitmapHopscotchHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestBitmapHopscotch(const Configuration& config) 
	: _ds(config.initial_count, config.no_of_threads) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		//return _ds.get(key);
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "BHOP";
	}
	void print() {}
	void shutdown() {}
};


class TestHopscotch_D : public TestDs {
private:
	HopscotchHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestHopscotch_D(const Configuration& config) 
		: _ds(config.initial_count, config.no_of_threads,64,true) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		//return _ds.get(key);
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "HOP_D";
	}
	void print() {}
	void shutdown() {}
};


class TestHopscotch_ND : public TestDs {
private:
	HopscotchHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestHopscotch_ND(const Configuration& config) 
		: _ds(config.initial_count, config.no_of_threads,64,false) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		//return _ds.get(key);
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "HOP_ND";
	}
	void print() {}
	void shutdown() {}
};


class TestChained_MTM : public TestDs {
private:
	ChainedHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestChained_MTM(const Configuration& config) : _ds(config.initial_count, config.no_of_threads,2.0, false) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "Chained_MTM";
	}
	void print() {}
	void shutdown() {}
};

class TestChained_PRE : public TestDs {
private:
	ChainedHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestChained_PRE(const Configuration& config) : _ds(config.initial_count, config.no_of_threads,2.0, true) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "Chained_PRE";
	}
	void print() {}
	void shutdown() {}
};

class TestChained_MMTM : public TestDs {
private:
	ChainedHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestChained_MMTM(const Configuration& config) : _ds(config.initial_count/2, config.no_of_threads,2.0, false) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "Chained_MTM";
	}
	void print() {}
	void shutdown() {}
};

class TestChained_MPRE : public TestDs {
private:
	ChainedHashMap<int, int, HASH_INT, TTASLock, CMDR::Memory> _ds;
public:
	TestChained_MPRE(const Configuration& config) : _ds(config.initial_count/22, config.no_of_threads,2.0, true) {
	}

	bool containsKey(int key) {
		return _ds.containsKey(key);
	}

	int get(int key) {
		return 0;
	}

	int put(int key, int value) {
		return _ds.putIfAbsent(key, value);
	}

	int remove(int key) {
		return _ds.remove(key);
	}

	int size() {
		return _ds.size();
	}
	const char* name() {
		return "Chained_PRE";
	}
	void print() {}
	void shutdown() {}
};

////////////////////////////////////////////////////////////////////////////////
//GLOBALS
////////////////////////////////////////////////////////////////////////////////
class BenchmarkThread;

CMDR::Random			_gRand;
Configuration			_gConfiguration;
int						_gNumProcessors;
int						_gNumThreads;
int						_gThroughputTime;

TestDs*					_gTestDs;
int						_gTotalRandNum;
int*						_gRandNumAry;
VolatileType<int>*	_gThreadResultAry;

BenchmarkThread*		_gThreads;
VolatileType<int>		_gIsStopThreads(0);
AtomicInteger			_gThreadEndCounter(0);	
AtomicInteger			_gThreadStartCounter(0);	
VolatileType<tick_t>	_gStartTime;
VolatileType<tick_t>	_gEndTime;

////////////////////////////////////////////////////////////////////////////////
//TYPEDEFS
////////////////////////////////////////////////////////////////////////////////
class BenchmarkThread: public Thread {
public:
	int state;
	int state2;
	int curr_counter;

	int _threadNo;
	int _i_start_suc_rand;
	int _i_end_suc_rand;
	int _i_start_unsuc_rand;
	int _i_end_unsuc_rand;
	int _num_add;
	int _num_remove;
	int _num_contain;
	static AtomicInteger _threadCounter;

	BenchmarkThread () : Thread() {
		_u64 volatile seed = Random::getSeed();
		_threadNo = _threadCounter.getAndIncrement();
		int num_actions = (int)((_gConfiguration.initial_count * _gConfiguration.load_factor )/ _gConfiguration.no_of_threads);

		_i_start_suc_rand		= _threadNo * num_actions;
		_i_end_suc_rand		= _i_start_suc_rand + num_actions;

		_i_start_unsuc_rand	= _i_start_suc_rand + _gConfiguration.initial_count;
		_i_end_unsuc_rand		= _i_end_suc_rand + _gConfiguration.initial_count;

		_num_add					= ((_gConfiguration.update_ops) / 2);
		_num_remove				= ((_gConfiguration.update_ops) / 2);
		_num_contain			= 100 - _num_add - _num_remove;

		state						= (Random::getRandom(seed,1023))&1;
		state2					= (Random::getRandom(seed,1023))&1;
		curr_counter			= _num_add;
		if(1==state2)
			curr_counter = _num_contain;
	}

	virtual void run() {
		//save start benchmark time 
		const int start_counter = _gThreadStartCounter.getAndIncrement();
		if(start_counter == (_gNumThreads-1))
			_gStartTime = System::currentTimeMillis();
		while((_gNumThreads) != _gThreadStartCounter.get()) {;}

		//execute thread benchmark
		unsigned int action_counter = 0;
		int i_suc = _i_start_unsuc_rand;
		int i_unsuc = _i_start_unsuc_rand;
		do {
			if(0 == state) {
				if(0==state2) {
					if(curr_counter>0){
						_gTestDs->remove(_gRandNumAry[i_suc]);
						_gTestDs->put(_gRandNumAry[i_unsuc], _gRandNumAry[i_unsuc]);
						action_counter+=2;

						++i_suc;
						++i_unsuc;
						if(i_suc >= _i_end_suc_rand) {
							state = 1;
							i_suc = _i_start_suc_rand;
							i_unsuc = _i_start_unsuc_rand;
						}
					}

					--curr_counter;
					if(curr_counter<=0) {
						state2=1;
						curr_counter = _num_contain;
					}
				} else {
					bool b1,b2;
					b1=_gTestDs->containsKey(_gRandNumAry[i_suc]);
					b2=_gTestDs->containsKey(_gRandNumAry[i_unsuc]);

					action_counter+=2;

					++i_suc;
					++i_unsuc;
					if(i_suc >= _i_end_suc_rand) {
						state = 1;
						i_suc = _i_start_suc_rand;
						i_unsuc = _i_start_unsuc_rand;
					}

					--curr_counter;
					if(curr_counter<=0) {
						state2=0;
						curr_counter = _num_add;
					}
				}
			} else if(1 == state) {
				if(0==state2) {
					if(curr_counter>0){
						_gTestDs->put(_gRandNumAry[i_suc], _gRandNumAry[i_suc]);
						_gTestDs->remove(_gRandNumAry[i_unsuc]);
						action_counter+=2;

						++i_suc;
						++i_unsuc;
						if(i_suc >= _i_end_suc_rand) {
							state = 0;
							i_suc = _i_start_suc_rand;
							i_unsuc = _i_start_unsuc_rand;
						}
					}

					--curr_counter;
					if(curr_counter<=0) {
						state2=1;
						curr_counter = _num_contain;
					}
				} else {
					bool b1,b2;
					b1=_gTestDs->containsKey(_gRandNumAry[i_suc]);
					b2=_gTestDs->containsKey(_gRandNumAry[i_unsuc]);
					action_counter+=2;

					++i_suc;
					++i_unsuc;
					if(i_suc >= _i_end_suc_rand) {
						state = 0;
						i_suc = _i_start_suc_rand;
						i_unsuc = _i_start_unsuc_rand;
					}

					--curr_counter;
					if(curr_counter<=0) {
						state2=0;
						curr_counter = _num_add;
					}
				}
			}

			//check if need to end benchmark
			if (0 != _gIsStopThreads)
				break;
		} while(true);

		//save end benchmark time
		const int end_counter = _gThreadEndCounter.getAndIncrement();
		if(end_counter == (_gNumThreads-1))
			_gEndTime = System::currentTimeMillis();
		while((_gNumThreads) != _gThreadEndCounter.get()) {Thread::yield();}

		//save thread benchmark result
		_gThreadResultAry[_threadNo] = action_counter;
	}
};

AtomicInteger BenchmarkThread::_threadCounter(0);

////////////////////////////////////////////////////////////////////////////////
//MAIN
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {

	//read benchmark configuration
	if(!_gConfiguration.read(argc, argv)) {
		if(!_gConfiguration.read()) {
			cerr << "USAGE: <algorithm> <testNum> <numThreads> <insertOps> <deleteOps> <loadFactor> <initialCount> <throughputTime>";
			exit(-1);
		}
	}
	Thread::set_concurency_level(_gConfiguration.no_of_threads+1);

	//initialize global variables
	_gNumProcessors     = 1;//Runtime.getRuntime().availableProcessors();
	_gNumThreads        = _gConfiguration.no_of_threads;
	_gTotalRandNum      = 2*(_gConfiguration.initial_count);
	_gThroughputTime    = _gConfiguration.throughputTime;

	//mainInstance.ThrSetConcurrency(_gConfiguration.no_of_threads+3);

	//run the benchmark
	unsigned int result = RunBenchmark();

	//print benchmark results
	if(0 != (_gConfiguration.is_print_result))
		cout <<  " " << result;// << endl;
	if(0 == (_gConfiguration.is_print_result))
		Thread::sleep(5*1000);

	return 0;
}

////////////////////////////////////////////////////////////////////////////
//HELPER FUNCTIONS
////////////////////////////////////////////////////////////////////////////
void PrepareRandomNumbers() {
	BitmapHopscotchHashMap<int, int, HASH_INT, DummyLock, CMDR::Memory> num_map(_gTotalRandNum*2, 1);

	_gRandNumAry = new int[_gTotalRandNum];
	int last_number =  1;
	for (int iRandNum = 0; iRandNum < _gTotalRandNum; ++iRandNum) {
		//last_number += (_gRand.nextInt(256) + 1);
		//last_number = _gRand.nextInt(256*1024*1024) + 1;
		last_number+=1;
		_gRandNumAry[iRandNum] = last_number;
		//cout<<"_gRandNumAry[%d]"<<iRandNum<<" = "<<_gRandNumAry[iRandNum] << endl;
	}
}

void FillTable(int table_size) {
	for (int iRandNum = 0; iRandNum < table_size; ++iRandNum) {
		_gTestDs->put(_gRandNumAry[iRandNum], _gRandNumAry[iRandNum]);
		//cerr << _gRandNumAry[iRandNum] << endl;
	}
	cerr << (string("    ") + _gTestDs->name() + " Num elm: " + Integer::toString(_gTestDs->size()))<< endl;
}

unsigned int RunBenchmark() {
	//print test information ...............................................
	cerr << "ConcurrentHashMap Benchmark"<< endl;
	cerr << "---------------------------"<< endl;
	cerr << ("    numOfThreads:   " + Integer::toString( _gConfiguration.no_of_threads)) << endl;
	cerr << ("    Algorithm Name: " + _gConfiguration.GetAlgName()) << endl;
	cerr << ("    NumProcessors:  " + Integer::toString(_gNumProcessors)) << endl;
	cerr << ("    testNo:         " + Integer::toString(_gConfiguration.test_no)) << endl;
	cerr << ("    insert ops:     " + Integer::toString((_gConfiguration.update_ops)/2)) << endl;
	cerr << ("    del ops:        " + Integer::toString((_gConfiguration.update_ops)/2)) << endl;
	cerr << ("    constain ops:   " + Integer::toString(100 - _gConfiguration.update_ops)) << endl;
	cerr << ("    loadFactor:     " + Integer::toString((int)(100*(_gConfiguration.load_factor)))) << endl;
	cerr << ("    initialCount:   " + Integer::toString(_gConfiguration.initial_count)) << endl;
	cerr << ("    throughputTime: " + Integer::toString(_gConfiguration.throughputTime)) << endl;

	//create appropriate hash-table ........................................
	if(0 == (_gConfiguration.GetAlgName().compare("hopd"))) {
		_gTestDs = new TestHopscotch_D(_gConfiguration);
	} else if(0 == (_gConfiguration.GetAlgName().compare("hopnd"))) {
		_gTestDs = new TestHopscotch_ND(_gConfiguration);
	} else if(0 == (_gConfiguration.GetAlgName().compare("bhop"))) {
		_gTestDs = new TestBitmapHopscotch(_gConfiguration);
	} else if(0 == (_gConfiguration.GetAlgName().compare("chainmtm"))) {
		_gTestDs = new TestChained_MTM(_gConfiguration);
	} else if(0 == (_gConfiguration.GetAlgName().compare("chainpre"))) {
		_gTestDs = new TestChained_PRE(_gConfiguration);
	} else if(0 == (_gConfiguration.GetAlgName().compare("chainmmtm"))) {
		_gTestDs = new TestChained_MMTM(_gConfiguration);
	} else if(0 == (_gConfiguration.GetAlgName().compare("chainmpre"))) {
		_gTestDs = new TestChained_MPRE(_gConfiguration);
	} else {
		cerr << "ERROR: unknown algorithm." << endl;
		exit(1);
	}
	cerr << ("    initialCapacity:   " + Integer::toString(_gConfiguration.initial_count))<< endl;

	//prepare the random numbers ...........................................
	cerr << endl;
	cerr << "    START create random numbers." << endl;
	PrepareRandomNumbers();
	cerr << "    END   creating random numbers."<< endl;
	cerr << endl;

	//fill the hash-table ..................................................
	int table_size  = (int)((_gConfiguration.initial_count) * (_gConfiguration.load_factor));
	cerr << ("    START fill table. (" + Integer::toString(table_size) + ")")<< endl;
	FillTable(table_size);
	cerr << "    END   fill table." << endl;
	cerr << endl;

	//create benchmark threads .............................................
	cerr << "    START creating threads." << endl;
	_gThreads = new BenchmarkThread[_gNumThreads];
	_gThreadResultAry = new  VolatileType<int>[_gNumThreads];
	Thread::sleep(_gThroughputTime*1000);
	cerr << "    END   creating threads." << endl;
	cerr << endl;

	//start the benchmark threads ..........................................
	cerr << "    START threads." << endl;
	for(int iThread = 0; iThread < _gNumThreads; ++iThread) {
		_gThreads[iThread].start();
	}
	cerr << "    END   threads." << endl;
	cerr << "" << endl;

	//Wait the throughput time, and then signal the threads to terminate ...
	cerr << "    STARTING test." << endl;
	if(0 != _gThroughputTime) {
		Thread::sleep(_gThroughputTime * 1000);
		_gIsStopThreads = 1;
		Memory::read_write_barrier();
	}
	cerr << "    ENDING test, ";

	//join the threads .....................................................
	for(int iThread = 0; iThread < _gNumThreads; ++iThread) {
		//cerr << "    Wait join  - " << iThread << endl;
		_gThreads[iThread].join();
	}
	cerr << "    ALL threads terminated."<< endl;
	//cerr << ""<< endl;

	//calculate threads results ............................................
	long long result = 0;
	for(int iThread = 0; iThread < _gNumThreads; ++iThread) {
		result += _gThreadResultAry[iThread];
	}

	//calculate total throughput
	long long throughput = 0.0;
	throughput = (result / (_gEndTime - _gStartTime)) / 1e3;
	cout<<"\tThroughput : "<<throughput <<" (Mops/s)\n"<< endl;
	//print benchmark results ..............................................
	cerr << ("    " + string(_gTestDs->name()) + " Num elm: " + Integer::toString(_gTestDs->size()))<< endl;

	//free resources .......................................................
	_gTestDs->shutdown();
	delete _gTestDs;
	delete [] _gRandNumAry;
	delete [] _gThreadResultAry;
	delete [] _gThreads;

	//return benchmark results .............................................
	if(0 == _gThroughputTime)
		return (unsigned int) (_gEndTime -  _gStartTime);
	else 
		return (unsigned int) (result / (_gThroughputTime*1000));
}
