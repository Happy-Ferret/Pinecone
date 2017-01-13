#pragma once

#include "Token.h"
#include "Action.h"
#include "ErrorHandler.h"

class NamespaceData;
typedef shared_ptr<NamespaceData> Namespace;

#include <vector>
using std::vector;

class AstNodeBase;
typedef unique_ptr<AstNodeBase> AstNode;

AstNode astNodeFromTokens(const vector<Token>&);

class AstNodeBase
{
public:
	
	void setInput(Namespace nsIn, bool dynamicIn, Type left, Type right)
	{
		if (inputHasBeenSet)
		{
			throw PineconeError("tried to set input on an AST node '"+getString()+"' more then once", INTERNAL_ERROR);
		}
		
		inputHasBeenSet=true;
		
		ns=nsIn;
		dynamic=dynamicIn;
		inLeftType=left;
		inRightType=right;
	}
	
	virtual bool isVoid() {return false;}
	virtual bool isType() {return false;}
	virtual bool isFunctionWithOutput() {return false;}
	
	virtual string getString()=0;
	
	Type getReturnType()
	{
		if (!returnType)
		{
			if (!inputHasBeenSet)
			{
				throw PineconeError("tried to get return type from AST node when input had not been set", INTERNAL_ERROR);
			}
			
			resolveReturnType();
		}
		
		return returnType;
	}
	
	Action getAction()
	{
		if (!action)
		{
			if (!ns)
			{
				throw PineconeError("tried to get action from AST node when input had not been set", INTERNAL_ERROR);
			}
			
			resolveAction();
		}
		
		return action;
	}
	
	void dealWithConstants()
	{
		if (!ns)
		{
			throw PineconeError("tried to deal with constants before input was set", INTERNAL_ERROR);
		}
		
		resolveConstant();
	}
	
protected:
	
	AstNodeBase() {}
	
	virtual void resolveReturnType()
	{
		returnType=getAction()->getReturnType();
	}
	
	virtual void resolveAction()=0;
	virtual void resolveConstant() {};
	
	Type inLeftType=nullptr, inRightType=nullptr;
	Action action=nullptr;
	Type returnType=nullptr;
	bool dynamic=false;
	Namespace ns=nullptr;
	bool inputHasBeenSet=false;
};

class AstVoid: public AstNodeBase
{
public:
	
	static unique_ptr<AstVoid> make() {return unique_ptr<AstVoid>(new AstVoid);}
	
	bool isVoid() {return true;}
	
	string getString() {return "void node";}
	
	void resolveReturnType() {returnType=Void;}
	
	void resolveAction()
	{
		if (!inLeftType->isVoid() || !inRightType->isVoid())
		{
			throw PineconeError("AstVoid given non void input", INTERNAL_ERROR);
		}
		
		action=voidAction;
	}
};
//extern AstNode astVoid;

class AstList: public AstNodeBase
{
public:
	
	//	make a new instance of this type of node
	static unique_ptr<AstList> make(vector<AstNode>& in)
	{
		unique_ptr<AstList> node(new AstList);
		node->nodes=move(in);
		return node;
	}
	
	string getString();
	
	//void resolveReturnType();
	
	void resolveAction();
	
private:
	
	//	the list of sub nodes
	vector<AstNode> nodes;
};

class AstConstExpression;

class AstToken: public AstNodeBase
{
public:
	
	static unique_ptr<AstToken> make(Token tokenIn)
	{
		unique_ptr<AstToken> node(new AstToken);
		node->token=tokenIn;
		return node;
	}
	
	string getString();
	
	void resolveAction();
	
private:
	
	friend AstConstExpression;
	
	Token token=nullptr;
};

class AstExpression: public AstNodeBase
{
public:
	
	static unique_ptr<AstExpression> make(AstNode leftInIn, AstNode centerIn, AstNode rightInIn)
	{
		unique_ptr<AstExpression> node(new AstExpression);
		
		node->leftIn=move(leftInIn);
		node->center=move(centerIn);
		node->rightIn=move(rightInIn);
		
		return node;
	}
	
	//bool isType() {return leftIn->isType() || rightIn->isType();}
	
	virtual string getString();
	
	virtual void resolveAction();
	
private:
	
	AstNode leftIn=nullptr, center=nullptr, rightIn=nullptr;
};

class AstConstExpression: public AstNodeBase
{
public:
	
	static unique_ptr<AstConstExpression> make(unique_ptr<AstToken> centerIn, AstNode rightInIn)
	{
		unique_ptr<AstConstExpression> node(new AstConstExpression);
		
		//node->leftIn=move(leftInIn);
		node->center=move(centerIn);
		node->rightIn=move(rightInIn);
		
		return node;
	}
	
	virtual string getString();
	
	void resolveConstant();
	void resolveAction() {action=voidAction;};
private:
	
	//AstNode leftIn=nullptr;
	unique_ptr<AstToken> center=nullptr;
	AstNode rightIn=nullptr;
};

class AstOpWithInput: public AstNodeBase
{
public:
	static unique_ptr<AstOpWithInput> make(vector<AstNode>& leftInIn, Token tokenIn, vector<AstNode>& rightInIn)
	{
		unique_ptr<AstOpWithInput> node(new AstOpWithInput);
		
		node->leftIn=move(leftInIn);
		node->token=tokenIn;
		node->rightIn=move(rightInIn);
		
		return node;
	}
	
	bool isFunctionWithOutput();
	
	string getString();
	
	void resolveAction();
	
	Token token=nullptr;
	vector<AstNode> leftIn, rightIn;
};

class AstTuple: public AstNodeBase
{
public:
	
	//	make a new instance of this type of node
	static unique_ptr<AstTuple> make(vector<AstNode>& in)
	{
		unique_ptr<AstTuple> node(new AstTuple);
		node->nodes=move(in);
		return node;
	}
	
	string getString();
	
	//void resolveReturnType();
	
	void resolveAction();
	
private:
	
	vector<AstNode> nodes;
};

class AstType: public AstNodeBase
{
public:
	bool isType() {return true;}
	
	void resolveAction()
	{
		throw PineconeError("AstType::resolveAction called, which it shouldn't have been", INTERNAL_ERROR);
	}
};

class AstVoidType: public AstType
{
public:
	
	static unique_ptr<AstVoidType> make()
	{
		unique_ptr<AstVoidType> node(new AstVoidType);
		return node;
	}
	
	string getString() {return "{}";}
	
	void resolveReturnType() {returnType=Void;}
	
private:
};

class AstTokenType: public AstType
{
public:
	
	static unique_ptr<AstTokenType> make(Token tokenIn)
	{
		unique_ptr<AstTokenType> node(new AstTokenType);
		node->token=tokenIn;
		return node;
	}
	
	string getString();
	
	void resolveReturnType();
	
private:
	
	Token token=nullptr;
};

class AstTupleType: public AstType
{
public:
	
	struct NamedType
	{
		Token name; // can be null
		unique_ptr<AstType> type;
	};
	
	static unique_ptr<AstTupleType> make(vector<NamedType>& in)
	{
		unique_ptr<AstTupleType> node(new AstTupleType);
		node->subTypes=move(in);
		return node;
	}
	
	string getString();
	
	void resolveReturnType();
	
private:
	
	vector<NamedType> subTypes;
};





