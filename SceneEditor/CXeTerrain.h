#pragma once

#include "IXeInc.h"

class CXeTerrain: public ObjectT<IXeTerrain>
{
public:
	CXeTerrain();
	~CXeTerrain();

	err_t Load(const char_16 * szFile);

	textw GetFile() const;
	textw GetName() const;
	textw GetNHeight() const { return m_nheight; }
	textw GetColor() const { return m_color; }
	textw GetLayer() const { return m_layer; }
	textw GetDetail() const { return m_detail; }
private:
	textw m_file;
	textw m_name;

	textw m_nheight;
	textw m_color;
	textw m_layer;
	textw m_detail;
};
