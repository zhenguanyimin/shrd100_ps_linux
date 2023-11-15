
#ifndef _GOC_SDK_H__
#define _GOC_SDK_H__

#ifdef __cplusplus
	extern "C" {
#endif


int goc_sdk_init();
int goc_sdk_init_ex(const char *path);
void goc_sdk_done();

#ifdef __cplusplus
}
#endif

#endif /* _GOC_SDK_H__ */




