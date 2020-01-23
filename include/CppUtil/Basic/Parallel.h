#pragma once
#ifndef _UBPA_CPPUTIL_BASIC_PARALLEL_H_
#define _UBPA_CPPUTIL_BASIC_PARALLEL_H_

#include <CppUtil/Basic/FunctionTraits.h>

#include <vector>
#include <thread>
#include <functional>

namespace Ubpa {
	class Parallel
	{
	public:
		static Parallel& Instance() {
			static Parallel instance;
			return instance;
		}

		const size_t & CoreNum() const { return coreNum; }
		template<typename Func>
		void Run(const std::vector<Func> & works);
		template<typename Func, typename Data>
		void Run(const Func & func, const std::vector<Data>& datas);
		template<typename Func>
		typename CppUtil::Basic::FunctionTraitsLambda<Func>::result_type RunSum(const std::vector<Func>& works);
		template<typename Func, typename Data>
		typename CppUtil::Basic::FunctionTraitsLambda<Func>::result_type RunSum(const Func& func, const std::vector<Data>& datas);
		
	private:
		Parallel();

		const size_t coreNum;
	};

	//------------------------------------------

	template<typename Func>
	void Parallel::Run(const std::vector<Func>& works) {
		std::vector<std::thread> workers;

		if (works.size() <= coreNum) {
			for (const auto& work : works)
				workers.emplace_back(work);
		}
		else {
			auto threadWork = [&works, workersNum = coreNum](size_t id) {
				for (size_t i = id; i < works.size(); i += workersNum)
					works[i]();
			};
			for (size_t i = 0; i < coreNum; i++)
				workers.emplace_back(threadWork, i);
		}

		for (auto& worker : workers)
			worker.join();
	}

	template<typename Func, typename Data>
	void Parallel::Run(const Func& func, const std::vector<Data>& datas) {
		std::vector<std::function<void()>> works;
		for (auto& data : datas)
			works.emplace_back([&]() { func(data); });
		Run(works);
	}

	template<typename Func>
	typename CppUtil::Basic::FunctionTraitsLambda<Func>::result_type Parallel::RunSum(const std::vector<Func>& works) {
		using RstType = typename CppUtil::Basic::FunctionTraitsLambda<Func>::result_type;
		std::vector<RstType> rsts;
		std::vector<std::thread> workers;

		if (works.size() <= coreNum) {
			for (size_t i = 0; i < works.size(); i++) {
				rsts.push_back(static_cast<RstType>(0));
				workers.emplace_back([&]() { rsts[i] = works[i](); });
			}
		}
		else {
			auto threadWork = [&works, &rsts, workersNum = coreNum](size_t id) {
				for (size_t i = id; i < works.size(); i += workersNum)
					rsts[id] += works[i]();
			};
			for (size_t i = 0; i < coreNum; i++) {
				rsts.push_back(static_cast<RstType>(0));
				workers.emplace_back(threadWork, i);
			}
		}

		for (auto& worker : workers)
			worker.join();

		auto sum = static_cast<RstType>(0);
		for (const auto& rst : rsts)
			sum += rst;
		return sum;
	}

	template<typename Func, typename Data>
	typename CppUtil::Basic::FunctionTraitsLambda<Func>::result_type Parallel::RunSum(const Func& func, const std::vector<Data>& datas) {
		using RstType = typename CppUtil::Basic::FunctionTraitsLambda<Func>::result_type;
		std::vector<std::function<RstType()>> works;
		for (auto& data : datas)
			works.emplace_back([&]() { return func(data); });
		return RunSum(works);
	}
}

#endif // !_UBPA_CPPUTIL_BASIC_PARALLEL_H_
