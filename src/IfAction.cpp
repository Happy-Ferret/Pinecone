#include "../h/IfAction.h"
#include "../h/ErrorHandler.h"

IfAction::IfAction(ActionPtr conditionIn, ActionPtr ifActionIn, string textIn)
	:Action(Void, Void, Void, textIn)
{
	condition=conditionIn;
	ifAction=ifActionIn;
	
	if (condition->getReturnType()!=Bool)
	{
		error.log("IfAction created with condition action that does not return Bool", INTERNAL_ERROR);
	}
	
	if (condition->getInLeftType()!=Void || condition->getInRightType()!=Void)
	{
		error.log("IfAction created with condition action that takes in something other then Void", INTERNAL_ERROR);
	}
	
	if (ifAction->getInLeftType()!=Void || ifAction->getInRightType()!=Void)
	{
		error.log("IfAction created with action that takes in something other then Void", INTERNAL_ERROR);
	}
}

string IfAction::getDescription()
{
	return "if " + condition->getDescription() + " then " + condition->getDescription();
}

void* IfAction::execute(void* inLeft, void* inRight)
{
	void* conditionOut=condition->execute(nullptr, nullptr);
	if (*((bool*)conditionOut))
		free(ifAction->execute(nullptr, nullptr));
	free(conditionOut);
	return nullptr;
}


