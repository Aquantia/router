#include "RuleController.h"
#include "Commands.h"
#include "LibBackend.h"

RuleController::RuleController()
{
	backEnd = new LibBackend();
}


std::unique_ptr<char[]> RuleController::addRules(const char * in, size_t  inSz)
{
	bool rc;
	AddRules _addRules(rc, in, inSz);	
	//TODO update cache
	std::shared_ptr<std::vector<rule>> r = _addRules.GetRules();
	rc &= backEnd->addRules(*r);
	
	return _addRules.constructResponse(rc);
}

std::unique_ptr<char[]> RuleController::updateRule(size_t id, const char * in, size_t  inSz) 
{
	bool rc;
	UpdateRule _updateRule (rc, in, inSz);
	
	int priority =  _updateRule.getPriority();
	rc &= backEnd->updateRule(id, priority);
	
	return _updateRule.constructResponse(rc);
}

std::unique_ptr<char[]> RuleController::getRule(size_t id) 
{
	bool rc;
	
	rule _r;
	rc &= backEnd->getRule(_r,id);
	
	GetRule getRule(_r);
	return getRule.constructResponse(rc);
}

std::unique_ptr<char[]>  RuleController::deleteRule(size_t id) 
{
	bool rc;
	
	rule _r;
	rc &= backEnd->getRule(_r, id);
	
	DeleteRule deleteRule;
	return deleteRule.constructResponse(rc);
}

std::unique_ptr<char[]> RuleController::getRules() 
{
	bool rc;
	
	std::vector<rule> vec = backEnd->scanRules(rc);
	
	GetRules getRules(vec);
	return getRules.constructResponse(rc);
}