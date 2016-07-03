/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include <rb/Array.h>
#include <rb/Log.h>

/*******************************************************/
/*              Defines                                */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestArray"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testArray() {
	if(!RB_CHECK_VERSION){
		RBLE("Invalid binary version");
		return -1;
	}

	int32_t rc;

	static const int kBFR_SIZE = 64;
	uint8_t bfr[kBFR_SIZE];

	Rb_ArrayHandle ar = Rb_Array_new();
	if(!ar){
		RBLE("Rb_Array_new failed");
		return -1;
	}

	if(Rb_Array_size(ar) != 0){
		RBLE("Rb_Array_size failed");
		return -1;
	}

	if(Rb_Array_tell(ar) != 0){
		RBLE("Rb_Array_tell failed");
		return -1;
	}

	// Write some data
	static const uint32_t kNUM_PASSES = 16;
	uint32_t i;
	for(i=0; i<kNUM_PASSES; i++){
		int32_t j;

		for(j=0; j<kBFR_SIZE; j++){
			bfr[j] = (i*kBFR_SIZE + j) % 0xFF;
		}

		rc = Rb_Array_write(ar, bfr, kBFR_SIZE);
		if(rc != kBFR_SIZE){
			RBLE("Rb_Array_write failed");
			return -1;
		}
	}

	// Verify data
	if(Rb_Array_size(ar) != kNUM_PASSES * kBFR_SIZE){
		RBLE("Rb_Array_size failed");
		return -1;
	}

	uint8_t* data = Rb_Array_data(ar);
	for(i=0; i<Rb_Array_size(ar); i++){
		if(i && data[i] != (data[i-1] + 1) % 0xFF){
			RBLE("Invalid data");
			return -1;
		}
	}

	rc = Rb_Array_free(&ar);
	if(rc != RB_OK || ar){
		RBLE("Rb_Array_free failed");
		return -1;
	}

	return 0;
}
