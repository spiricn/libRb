/*******************************************************/
/*              Includes                               */
/*******************************************************/

#include "TestCommon.h"

#include <rb/ConsumerProducer.h>
#include <rb/Log.h>
#include <pthread.h>
#include <stdbool.h>

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

#ifdef RB_LOG_TAG
#undef RB_LOG_TAG
#endif
#define RB_LOG_TAG "TestConsumerProducer"

/*******************************************************/
/*              Functions Definitions                  */
/*******************************************************/

int testConsProd() {
    int32_t rc;

    ASSERT_EQUAL(RB_TRUE, RB_CHECK_VERSION);

    Rb_ConsumerProducerHandle cp = Rb_ConsumerProducer_new();
    ASSERT_NOT_NULL(cp);

    rc = Rb_ConsumerProducer_free(&cp);
    ASSERT_EQUAL(RB_OK, rc);
    ASSERT_NULL(cp);

    return 0;
}
