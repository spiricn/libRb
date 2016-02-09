/**
 * @file MessageBox.h
 * @author Nikola Spiric <nikola.spiric@rt-rk.com>
 */

#ifndef MESSAGEBOX_H_
#define MESSAGEBOX_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"{
#endif

struct MessageBox_t;

typedef struct MessageBox_t* MessageBox;

MessageBox MessageBox_new(int32_t messageSize, int32_t capacity);

int32_t MessageBox_free(MessageBox* mb);

int32_t MessageBox_read(MessageBox mb, void* message);

int32_t MessageBox_write(MessageBox mb, const void* message);

int32_t MessageBox_getNumMessages(MessageBox mb);

#ifdef __cplusplus
}
#endif


#endif /* MESSAGEBOX_H_ */


