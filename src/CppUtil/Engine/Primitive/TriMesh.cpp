#include <CppUtil/Engine/TriMesh.h>

#include <CppUtil/Engine/Triangle.h>

#include <CppUtil/Basic/Cube.h>
#include <CppUtil/Basic/Sphere.h>
#include <CppUtil/Basic/Plane.h>
#include <CppUtil/Basic/Disk.h>
#include <CppUtil/Basic/BasicSampler.h>
#include <CppUtil/Basic/Parallel.h>

#include <mutex>
#include <map>

using namespace CppUtil;
using namespace CppUtil::Engine;
using namespace CppUtil::Basic;
using namespace std;
using namespace Ubpa;

TriMesh::TriMesh(uint triNum, uint vertexNum,
	const uint * indice,
	const float * positions,
	const float * normals,
	const float * texcoords,
	const float * tangents,
	ENUM_TYPE type)
	: type(type)
{
	if (!indice || !positions || !texcoords) {
		type = ENUM_TYPE::INVALID;
		printf("ERROR: TriMesh is invalid.\n");
		return;
	}

	for (uint i = 0; i < vertexNum; i++) {
		this->positions.push_back(Point3(positions[3 * i], positions[3 * i + 1], positions[3 * i + 2]));
		if(normals)
			this->normals.push_back(Normalf(normals[3 * i], normals[3 * i + 1], normals[3 * i + 2]));
		if(texcoords)
			this->texcoords.push_back(Point2(texcoords[2 * i], texcoords[2 * i + 1]));
		if(tangents)
			this->tangents.push_back({ tangents[3 * i],tangents[3 * i + 1],tangents[3 * i + 2] });
	}

	// traingel �� mesh �� init ��ʱ������
	// ��Ϊ���ڻ�û������ share_ptr
	for (uint i = 0; i < triNum; i++) {
		this->indice.push_back(indice[3 * i]);
		this->indice.push_back(indice[3 * i + 1]);
		this->indice.push_back(indice[3 * i + 2]);

		triangles.push_back(Triangle::New(indice[3 * i], indice[3 * i + 1], indice[3 * i + 2]));
	}

	if (!texcoords)
		this->texcoords.resize(vertexNum);

	if (!normals)
		GenNormals();

	if (!tangents) {
		if (texcoords)
			GenTangents();
		else
			this->tangents.resize(vertexNum);
	}
}

void TriMesh::Init(bool creator, const std::vector<uint> & indice,
	const std::vector<Point3> & positions,
	const std::vector<Normalf> & normals,
	const std::vector<Point2> & texcoords,
	const std::vector<Normalf> & tangents,
	ENUM_TYPE type)
{
	this->indice.clear();
	this->positions.clear();
	this->normals.clear();
	this->texcoords.clear();
	triangles.clear();
	this->type = ENUM_TYPE::INVALID;

	if (!(indice.size() > 0 && indice.size() % 3 == 0)
		|| positions.size() <= 0
		|| !normals.empty() && normals.size() != positions.size()
		|| !texcoords.empty() && texcoords.size() != positions.size()
		|| (tangents.size() != 0 && tangents.size() != positions.size()))
	{
		printf("ERROR: TriMesh is invalid.\n");
		return;
	}

	this->indice = indice;
	this->positions = positions;
	this->type = type;

	// traingel �� mesh �� init ��ʱ������
	// ��Ϊ���ڻ�û������ share_ptr
	for (size_t i = 0; i < indice.size(); i += 3)
		triangles.push_back(Triangle::New(indice[i], indice[i + 1], indice[i + 2]));

	if (texcoords.empty())
		this->texcoords.resize(positions.size());
	else
		this->texcoords = texcoords;

	if (normals.empty())
		GenNormals();
	else
		this->normals = normals;

	if (tangents.size() == 0) {
		if (texcoords.empty())
			this->tangents.resize(positions.size());
		else
			GenTangents();
	}
	else
		this->tangents = tangents;

	if (!creator)
		Init_AfterGenPtr();
}

bool TriMesh::Update(const std::vector<Point3> & positions) {
	if (type == INVALID) {
		printf("ERROR::TriMesh::Update:\n"
			"\t""type == INVALID\n");
		return false;
	}

	if (positions.size() != this->positions.size()) {
		printf("ERROR::TriMesh::Update:\n"
			"\t""%zd positions.size() != %zd this->positions.size()\n", positions.size(), this->positions.size());
		return false;
	}

	this->positions = positions;

	return true;
}

bool TriMesh::Update(const vector<Point2> & texcoords) {
	if (type == INVALID) {
		printf("ERROR::TriMesh::Update:\n"
			"\t""type == INVALID\n");
		return false;
	}

	if (texcoords.size() != positions.size()) {
		printf("ERROR::TriMesh::Update:\n"
			"\t""%zd texcoords.size() != %zd positions.size()\n", texcoords.size(), positions.size());
		return false;
	}

	this->texcoords = texcoords;

	return true;
}

void TriMesh::Init_AfterGenPtr() {
	auto triMesh = This<TriMesh>();
	for (auto triangle : triangles)
		triangle->mesh = triMesh;

	for (auto triangle : triangles)
		box.UnionWith(triangle->GetBBox());
}

void TriMesh::GenNormals() {
	normals.clear();
	normals.resize(positions.size(), Normalf(0.f));

	vector<mutex> vertexMutexes(positions.size());
	auto calSWN = [&](Ptr<Triangle> triangle) {
		auto v0 = triangle->idx[0];
		auto v1 = triangle->idx[1];
		auto v2 = triangle->idx[2];
		
		auto d10 = positions[v0] - positions[v1];
		auto d12 = positions[v2] - positions[v1];
		auto wN = d12.Cross(d10);
		
		for (size_t i = 0; i < 3; i++) {
			auto v = triangle->idx[i];
			vertexMutexes[v].lock();
			normals[v] += wN;
			vertexMutexes[v].unlock();
		}
	};
	auto calN = [&](size_t v) { normals[v].NormalizeSelf(); };
	Parallel::Instance().Run(calSWN, triangles);
	Parallel::Instance().Run(calN, positions.size());
}

void TriMesh::GenTangents() {
	const size_t vertexNum = positions.size();
	const size_t triangleCount = indice.size() / 3;

	vector<Normalf> tanS(vertexNum);
	vector<Normalf> tanT(vertexNum);
	vector<mutex> vertexMutexes(vertexNum);
	auto calST = [&](Ptr<Triangle> triangle) {
		auto i1 = triangle->idx[0];
		auto i2 = triangle->idx[1];
		auto i3 = triangle->idx[2];

		const Point3& v1 = positions[i1];
		const Point3& v2 = positions[i2];
		const Point3& v3 = positions[i3];

		const Point2& w1 = texcoords[i1];
		const Point2& w2 = texcoords[i2];
		const Point2& w3 = texcoords[i3];

		float x1 = v2.x - v1.x;
		float x2 = v3.x - v1.x;
		float y1 = v2.y - v1.y;
		float y2 = v3.y - v1.y;
		float z1 = v2.z - v1.z;
		float z2 = v3.z - v1.z;

		float s1 = w2.x - w1.x;
		float s2 = w3.x - w1.x;
		float t1 = w2.y - w1.y;
		float t2 = w3.y - w1.y;

		float denominator = s1 * t2 - s2 * t1;
		float r = denominator == 0.f ? 1.f : 1.f / denominator;
		Normalf sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
			(t2 * z1 - t1 * z2) * r);
		Normalf tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
			(s1 * z2 - s2 * z1) * r);

		for (size_t i = 0; i < 3; i++) {
			auto v = triangle->idx[i];
			vertexMutexes[v].lock();
			tanS[v] += sdir;
			tanT[v] += tdir;
			vertexMutexes[v].unlock();
		}
	};
	Parallel::Instance().Run(calST, triangles);

	tangents.resize(vertexNum);
	auto calTan = [&](size_t i) {
		const Normalf& n = normals[i];
		const Normalf& t = tanS[i];

		// Gram-Schmidt orthogonalize
		auto projT = t - n * n.Dot(t);
		tangents[i] = projT.Norm2() == 0.f ? BasicSampler::UniformOnSphere() : projT.Normalize();

		// Calculate handedness
		tangents[i] *= (n.Cross(t).Dot(tanT[i]) < 0.0F) ? -1.0F : 1.0F;
	};
	Parallel::Instance().Run(calTan, vertexNum);
}

const Ptr<TriMesh> TriMesh::GenCube() {
	Cube cube;
	auto cubeMesh = TriMesh::New(cube.GetTriNum(), cube.GetVertexNum(),
		cube.GetIndexArr(), cube.GetPosArr(), cube.GetNormalArr(), cube.GetTexCoordsArr(), nullptr, ENUM_TYPE::CUBE);
	return cubeMesh;
}

const Ptr<TriMesh> TriMesh::GenSphere() {
	Sphere sphere(50);
	auto sphereMesh = TriMesh::New(sphere.GetTriNum(), sphere.GetVertexNum(),
		sphere.GetIndexArr(), sphere.GetPosArr(), sphere.GetNormalArr(), sphere.GetTexCoordsArr(), sphere.GetTangentArr(), ENUM_TYPE::SPHERE);
	return sphereMesh;
}

const Ptr<TriMesh> TriMesh::GenPlane() {
	Plane plane;
	auto planeMesh = TriMesh::New(plane.GetTriNum(), plane.GetVertexNum(),
		plane.GetIndexArr(), plane.GetPosArr(), plane.GetNormalArr(), plane.GetTexCoordsArr(), nullptr, ENUM_TYPE::PLANE);
	return planeMesh;
}

const Ptr<TriMesh> TriMesh::GenDisk() {
	Disk disk(50);
	auto diskMesh = TriMesh::New(disk.GetTriNum(), disk.GetVertexNum(),
		disk.GetIndexArr(), disk.GetPosArr(), disk.GetNormalArr(), disk.GetTexCoordsArr(), disk.GetTangentArr(), ENUM_TYPE::DISK);
	return diskMesh;
}
