#ifndef _CPPUTIL_ENGINE_MESHEDIT_ISOTROPICREMESHING_H_
#define _CPPUTIL_ENGINE_MESHEDIT_ISOTROPICREMESHING_H_

#include <CppUtil/Basic/HeapObj.h>
#include <CppUtil/Basic/UGM/UGM.h>

#include <3rdParty/HEMesh/HEMesh.h>

namespace CppUtil {
	namespace Engine {
		class TriMesh;

		class IsotropicRemeshing : public Basic::HeapObj {
		public:
			IsotropicRemeshing(Basic::Ptr<TriMesh> triMesh);

		public:
			static const Basic::Ptr<IsotropicRemeshing> New(Basic::Ptr<TriMesh> triMesh) {
				return Basic::New<IsotropicRemeshing>(triMesh);
			}

		public:
			bool Init(Basic::Ptr<TriMesh> triMesh);
			void Clear();
			bool Run(size_t n);

		private:
			bool Kernel(size_t n);

		private:
			class V;
			class E;
			class V : public Ubpa::TVertex<V, E> {
			public:
				V(const Vec3 pos = 0.f) : pos(pos) {}
			public:
				const Vec3 Project(const Vec3& p, const Normalf & norm) const;
			public:
				Vec3 pos;
				Vec3 newPos;
			};
			class E : public Ubpa::TEdge<V, E> {
			public:
				float Length() const { return (HalfEdge()->Origin()->pos - HalfEdge()->End()->pos).Norm(); }
				Vec3 Centroid() const { return (HalfEdge()->Origin()->pos + HalfEdge()->End()->pos) / 2.f; }
			};
		private:
			Basic::Ptr<TriMesh> triMesh;
			const Basic::Ptr<Ubpa::HEMesh<V>> heMesh;
		};
	}
}

#endif // !_CPPUTIL_ENGINE_MESHEDIT_ISOTROPICREMESHING_H_
