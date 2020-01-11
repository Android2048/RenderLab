#include <CppUtil/Basic/HEMesh/HEMesh.h>
#include <CppUtil/Basic/UGM/UGM.h>
#include <iostream>
#include <string>

using namespace CppUtil::Basic;
using namespace std;
using namespace CppUtil;

class V;
class E;
class P;
class V : public TVertex<V,E,P> {
public:
	V(const string & name = "NO_NAME") : name(name){}
protected:
	virtual ~V() = default;
public:
	string name;
};
class E : public TEdge<V, E, P> {
public:
	E(const string& pre = "E") : pre(pre){}
protected:
	virtual ~E() = default;
public:
	const string Name() const { return "[" + pre + "]" + HalfEdge()->Origin()->name + "-" + HalfEdge()->End()->name; }
private:
	string pre;
};
class P : public TPolygon<V, E, P> {
public:
	P(const string& pre = "P") :pre(pre) {}
public:
	const string Name() const {
		string name = "[" + pre + "]";
		auto he = HalfEdge();
		name += he->Origin()->name;
		do {
			name += "-" + he->End()->name;
			he = he->Next();
		} while (he != HalfEdge());
		return name;
	}
protected:
	virtual ~P() = default;
private:
	string pre;
};

ostream & operator<< (ostream & os, Ptr<V> v) {
	os << v->name;
	return os;
}

ostream & operator<< (ostream & os, Ptr<HEMesh<V>::HE> he) {
	os << he->Origin() << "->" << he->End()
		<< ", next " << he->Next()->Origin() << "->" << he->Next()->End()
		<< ", pair " << he->Pair()->Origin() << "->" << he->Pair()->End();
	return os;
}

void Print(Ptr<HEMesh<V>> mesh) {
	cout << " V:" << mesh->Vertices().size() << endl;
	for (auto v : mesh->Vertices())
		cout << "    "<< v->name << endl;

	cout << "HE:" << mesh->HalfEdges().size() << endl;
	for (auto he : mesh->HalfEdges())
		cout << "    " << he << endl;

	cout << " E:" << mesh->Edges().size() << endl;
	for (auto e : mesh->Edges())
		cout << "    " << e->Name() << endl;

	cout << " P:" << mesh->Polygons().size() << endl << endl;
	for (auto p : mesh->Polygons())
		cout << "    " << p->Name() << endl;

	cout << endl;
}

int main() {
	// test basic
	{
		cout
			<< "------------------" << endl
			<< "    test basic    " << endl
			<< "------------------" << endl;

		auto mesh = HEMesh<V>::New();
		cout << "add v0, v1, v2" << endl;

		auto v0 = mesh->AddVertex("v0");
		auto v1 = mesh->AddVertex("v1");
		auto v2 = mesh->AddVertex("v2");
		Print(mesh);

		cout << "add e01, e12, e21" << endl;
		auto e01 = mesh->AddEdge(v0, v1, "E0");
		auto e12 = mesh->AddEdge(v1, v2, "E1");
		auto e20 = mesh->AddEdge(v2, v0, "E2");
		Print(mesh);

		cout << "add p0" << endl;
		auto p0 = mesh->AddPolygon({ e01->HalfEdge(),e12->HalfEdge(),e20->HalfEdge() }, "P0");
		Print(mesh);

		cout << e01->Name() << " is " << (e01->IsBoundary() ? "" : "not ") << "a boundary" << endl;
		cout << v0->name << " is " << (v0->IsBoundary() ? "" : "not ") << "a boundary" << endl;

		cout << "remove p0" << endl;
		mesh->RemovePolygon(p0);
		Print(mesh);

		cout << "remove e01" << endl;
		mesh->RemoveEdge(e01);
		Print(mesh);

		cout << "remove v0" << endl;
		mesh->RemoveVertex(v0);
		Print(mesh);
	}
	
	// test spilt edge
	{
		cout
			<< "----------------------" << endl
			<< "    test SpiltEdge    " << endl
			<< "----------------------" << endl;

		auto mesh = HEMesh<V>::New();

		auto v0 = mesh->AddVertex("v0");
		auto v1 = mesh->AddVertex("v1");
		auto v2 = mesh->AddVertex("v2");
		auto v3 = mesh->AddVertex("v3");

		auto e01 = mesh->AddEdge(v0, v1, "E0");
		auto e12 = mesh->AddEdge(v1, v2, "E1");
		auto e02 = mesh->AddEdge(v0, v2, "E2");
		auto e23 = mesh->AddEdge(v2, v3, "E3");
		auto e30 = mesh->AddEdge(v3, v0, "E4");

		auto he01 = e01->HalfEdge();
		auto he12 = e12->HalfEdge();
		auto he02 = e02->HalfEdge();
		auto he23 = e23->HalfEdge();
		auto he30 = e30->HalfEdge();
		auto he20 = he02->Pair();

		mesh->AddPolygon({ he01,he12,he20 }, "P0");
		mesh->AddPolygon({ he02,he23,he30 }, "P1");

		auto v = mesh->SpiltEdge(e02);
		v->name = "v4";

		Print(mesh);
	}

	// test rotate edge
	{
		cout
			<< "-----------------------" << endl
			<< "    test RotateEdge    " << endl
			<< "-----------------------" << endl;

		auto mesh = HEMesh<V>::New();

		auto v0 = mesh->AddVertex("v1");
		auto v1 = mesh->AddVertex("v2");
		auto v2 = mesh->AddVertex("v3");
		auto v3 = mesh->AddVertex("v4");

		auto e01 = mesh->AddEdge(v0, v1, "E0");
		auto e12 = mesh->AddEdge(v1, v2, "E1");
		auto e02 = mesh->AddEdge(v0, v2, "E2");
		auto e23 = mesh->AddEdge(v2, v3, "E3");
		auto e30 = mesh->AddEdge(v3, v0, "E4");

		auto he01 = e01->HalfEdge();
		auto he12 = e12->HalfEdge();
		auto he02 = e02->HalfEdge();
		auto he23 = e23->HalfEdge();
		auto he30 = e30->HalfEdge();
		auto he20 = he02->Pair();

		mesh->AddPolygon({ he01,he12,he20 }, "P0");
		mesh->AddPolygon({ he02,he23,he30 }, "P1");

		Print(mesh);
		mesh->RotateEdge(e02);
		Print(mesh);
	}

	// test collapse edge
	{
		cout
			<< "-------------------------" << endl
			<< "    test CollapseEdge    " << endl
			<< "-------------------------" << endl;

		auto mesh = HEMesh<V>::New();

		auto v0 = mesh->AddVertex("v0");
		auto v1 = mesh->AddVertex("v1");
		auto v2 = mesh->AddVertex("v2");
		auto v3 = mesh->AddVertex("v3");

		auto e01 = mesh->AddEdge(v0, v1, "E0");
		auto e12 = mesh->AddEdge(v1, v2, "E1");
		auto e02 = mesh->AddEdge(v0, v2, "E2");
		auto e23 = mesh->AddEdge(v2, v3, "E3");
		auto e30 = mesh->AddEdge(v3, v0, "E4");

		auto he01 = e01->HalfEdge();
		auto he12 = e12->HalfEdge();
		auto he02 = e02->HalfEdge();
		auto he23 = e23->HalfEdge();
		auto he30 = e30->HalfEdge();
		auto he20 = he02->Pair();

		mesh->AddPolygon({ he01,he12,he20 }, "P0");
		mesh->AddPolygon({ he02,he23,he30 }, "P1");

		auto newV = mesh->SpiltEdge(e02);

		Print(mesh);
	
		auto p = mesh->EraseVertex(newV);

		Print(mesh);

		mesh->AddPolygonVertex(p, v1, "v5");

		Print(mesh);
	}

	return 0;
}
