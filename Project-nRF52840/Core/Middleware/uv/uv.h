#ifndef _UV_H_
#define _UV_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void uv_init(void);
void uv_get_data(uint8_t *uv_index);

#ifdef __cplusplus
}
#endif

#endif /* _UV_H_ */