#pragma once

#include "AbstractRuleController.h"
#include "AbstractBackEnd.h"

class RuleController : public AbstractRuleController
{
public:
	RuleController();
	std::unique_ptr<char[]> getRules() override;
	std::unique_ptr<char[]> addRules(const char * in, size_t  inSz)  override;

	std::unique_ptr<char[]> updateRule(size_t id, const char * in, size_t  inSz) override;
	std::unique_ptr<char[]> getRule(size_t id) override;
	std::unique_ptr<char[]> deleteRule(size_t id) override;
private:
	AbstractBackEnd * backEnd;
};