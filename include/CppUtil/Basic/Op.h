#ifndef _CPPUTIL_BASIC_OPERATION_OP_H_
#define _CPPUTIL_BASIC_OPERATION_OP_H_

namespace CppUtil {
	namespace Basic {
		class Op {
		protected:
			Op(bool isHold = true) :isHold(isHold) { }
			virtual ~Op() = default;

		public:
			void operator()() { Run(); }
			virtual void Run() = 0;

		public:
			bool isHold;
		};
	}
}

#endif//! _CPPUTIL_BASIC_OPERATION_OP_H_
