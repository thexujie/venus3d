#include "stdafx.h"
#include "venus3d_direct11.h"

#include "CDx11VedioDriver.h"
#include "CDxInputDriver.h"

VENUS_BEG

I3DFileDriver * CreateFileDriver_Direct11()
{
	I3DFileDriver * pDriver = new C3DFileDriver();
	err_t err = pDriver->Initialize();
	if(err)
		SafeRelease(pDriver);
	return pDriver;
}

I3DVedioDriver * CreateVedioDriver_Direct11()
{
	I3DVedioDriver * pDriver =  new CDx11VedioDriver();
	err_t err = pDriver->Initialize();
	if(err)
		SafeRelease(pDriver);
	return pDriver;
}

I3DInputDriver * CreateInputDriver_Direct11()
{
	I3DInputDriver * pDriver = new CDxInputDriver();
	err_t err = pDriver->Initialize();
	if(err)
		SafeRelease(pDriver);
	return pDriver;
}
VENUS_END
