#pragma once

#include "BaseInc.h"

class IXeObject : public IObject
{

};

class IXeTerrain : public IXeObject
{
public:
	virtual textw GetFile() const = 0;
	virtual textw GetName() const = 0;
	virtual textw GetNHeight() const = 0;
	virtual textw GetColor() const = 0;
	virtual textw GetLayer() const = 0;
	virtual textw GetDetail() const = 0;
};

class IXeScene : public IXeObject
{

};

class IXeResources : public IXeObject
{
public:
	virtual vector<IXeTerrain *> & GetTerrains() = 0;
};

class IXeSolution : public IXeObject
{
public:
	virtual IXeResources * GetResources() = 0;
	virtual err_t Load(const char_16 * szFile) = 0;
};

IXeSolution * CreateXeSolution();
