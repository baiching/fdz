#include "task_queue.h"
#include "concurrentqueue.h"


moodycamel::ConcurrentQueue<std::unique_ptr<batch_s>> q;

void add_batch(std::unique_ptr<batch_s> batch) {
	q.enqueue(std::move(batch));
}
std::unique_ptr<batch_s> get_batch() {
	std::unique_ptr<batch_s> s;
	if (q.try_dequeue(s)) {
		return s;
	}

	return nullptr;
}