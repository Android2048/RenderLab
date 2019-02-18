#include <CppUtil/Engine/BSDF_Diffuse.h>
#include <CppUtil/Basic/Math.h>

using namespace CppUtil::Engine;
using namespace CppUtil::Basic;
using namespace glm;

vec3 BSDF_Diffuse::F(const vec3 & wo, const vec3 & wi, const vec2 & texcoord) {
	return albedo / Math::PI;
}

vec3 BSDF_Diffuse::Sample_f(const vec3 & wo, const vec2 & texcoord, vec3 & wi, float & PD) {
	wi = sampler.GetSample(PD);

	return albedo / Math::PI;
}

float BSDF_Diffuse::PDF(const vec3 & wo, const vec3 & wi, const vec2 & texcoord) {
	return wi.z / Math::PI;
}
