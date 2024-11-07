
#ifndef DRIVER_INTERFACE_H
#define DRIVER_INTERFACE_H

typedef struct driver_s
{
	struct driver_s* handle;//Á¾¼ÓµÈ driver
	int  num;
	void* sem;
	const void* api;
	void* cfg;
}driver_t;



#endif
