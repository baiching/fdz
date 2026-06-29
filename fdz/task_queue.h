#pragma once
#include <vector>
#include <memory>
#include <string>

typedef struct {
	std::vector<std::string> data;
	size_t s;
} batch_s;

void add_batch(std::unique_ptr<batch_s> batch);
std::unique_ptr<batch_s> get_batch();