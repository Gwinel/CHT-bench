	.inline MEMBAR_ALL, 0
membar	#StoreLoad
nop
	.end

	.inline MEMBAR_STORESTORE, 0
membar	#StoreLoad
nop
	.end

	.inline MEMBAR_LOADLOAD, 0
nop
	.end

	.inline CASPO_internal
casx [%o0], %o1, %o2
mov %o2, %o0
	.end
	
	.inline CAS64O_internal
casx [%o0], %o1, %o2
mov %o2, %o0
	.end

	.inline CASIO_internal
cas [%o0], %o1, %o2
mov %o2, %o0
	.end

	.inline RDTICK, 0
rd %tick, %o0
	.end

	.inline POPC, 1
popc %o0, %o1
mov %o1, %o0
	.end
