#include <ThreadManager/header/ThreadManager.h>
#include <SharedTools/header/library_loader.h>
#include <iostream>
#include <thread>
#include <chrono>

using namespace thread_management;
using namespace library_loader;
using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
	TModuleLoader<CThreadManager> threadManagerLoader(L"ThreadManager");
	auto threadManager = threadManagerLoader.New();
	threadManager->InitializeThreadCount(8);
	//auto task0 = threadManager->NewTask()
	//	->Name("Test Work 0")
	//	->Functor([]()
	//		{
	//			std::cout << "0";
	//		});
	auto graph = threadManager->NewTaskGraph()
		->Name("Test Work Group 0");
		//->DependsOn(task0);
			
	auto task1 = graph->NewTask()
		->Name("Test Work 1")
		->Functor([]()
			{
				std::cout << "1";
			});
	auto task2 = graph->NewTask()
		->Name("Test Work 2")
		->Functor([]()
			{
				std::this_thread::sleep_for(2000ms);
				std::cout << "2";
			})
		->DependsOn(task1);
			auto graph01 = graph->NewTaskGraph()
				->Name("Test Work Group 01")
				 ->DependsOn(task2);
	graph01->NewTask()
		->Name("Test Work 3")
		->Functor([]()
			{
				std::this_thread::sleep_for(1000ms);
				std::cout << "3";
			});
	graph01->NewTask()
		->Name("Test Work 4")
		->Functor([threadManager]()
			{
				std::cout << "4";

				auto graph02 = threadManager->NewTaskGraph()
					->Name("Test Work Group 02");
				graph02->NewTask()
					->Name("Test Work 5")
					->Functor([]()
						{
							std::cout << "5";
						});
				graph02->Run();
			});
	graph01->NewTaskParallelFor()
		->Name("Test Work Parallel For")
			->Functor([](uint32_t i)
			{
				std::cout << ("p" + std::to_string(i));
			})
		->JobCount(10);
			auto setupTask = graph01->NewTask()
				->Name("Setup Subgraph0");
			auto testSubgraph = graph01->NewTaskGraph()->Name("Delay Setup Taskgraph")->DependsOn(setupTask);

			setupTask->Functor([testSubgraph]()
				{
					std::cout << "delay setup";
					testSubgraph->NewTask()->Name("delay subtask0")->Functor([]()
						{
							std::this_thread::sleep_for(20ms);
							std::cout << " ds0";
						});
					testSubgraph->NewTask()->Name("delay subtask1")->Functor([]()
						{
							std::this_thread::sleep_for(20ms);
							std::cout << " ds1";
						});
				});
	auto future = graph->Run();
	future.wait();
	return 0;
}