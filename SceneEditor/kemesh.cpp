#include "stdafx.h"
#include "kemesh.h"

void kemesh_load(kemesh_t & mesh, const char_16 * szFile)
{
	xml_doc doc;
	err_t err = doc.load_file(szFile);

	mesh.pos_min = doc[L"pos_min"].value.totype(texttofloat3, L' ');
	mesh.pos_max = doc[L"pos_max"].value.totype(texttofloat3, L' ');
	mesh.tex_min = doc[L"tex_min"].value.totype(texttofloat2, L' ');
	mesh.tex_max = doc[L"tex_max"].value.totype(texttofloat2, L' ');

	const xml_node & node_vertices = doc(L"vertices");
	const xml_node & node_triangles = doc(L"triangles");

	mesh.vertices.resize(node_vertices.children.size());
	mesh.indices.resize(node_triangles.children.size() * 3);

	for(int_x cnt = 0; cnt < node_vertices.children.size(); ++cnt)
	{
		const xml_node & node = node_vertices.children[cnt];
		const textw & pos = node[L"pos"].value;
		const textw & tex = node[L"tex"].value;
		const textw & tan = node[L"tangent"].value;
		kevertex_t & vertex = mesh.vertices[cnt];
		texttofarr(pos.buffer(), pos.length(), &vertex.pos.af[0], 3, L' ');
		texttofarr(tex.buffer(), tex.length(), &vertex.tex.af[0], 2, L' ');
		texttofarr(tan.buffer(), tan.length(), &vertex.tan.af[0], 4, L' ');
	}

	for(int_x cnt = 0; cnt < node_triangles.children.size(); ++cnt)
	{
		const xml_node & node = node_triangles.children[cnt];
		const textw & index = node[L"index"].value;
		texttoiarr(index.buffer(), index.length(), &mesh.indices[cnt * 3], 3, L' ');
	}
}
