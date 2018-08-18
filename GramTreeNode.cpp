#include "Scanner.h"
#include "GramTreeNode.h"

#include "Visitor.h"


SyntaxTreeNodeBase* SyntaxTreeNodeBase::s_curVarDecType = nullptr;
stack<ClassTreeNode*> SyntaxTreeNodeBase::s_stackCurClassZone;
stack<SubroutineDecNode*> SyntaxTreeNodeBase::s_stackCurSubroutineZone;
stack<SubroutineBodyNode*> SyntaxTreeNodeBase::s_stackCurSubroutineBodyZone;
stack<CompondStatement*> SyntaxTreeNodeBase::s_stackCurCompoundStatmentZone;
int SyntaxTreeNodeBase::s_nCurNodeIndex = 0;

void SyntaxTreeNodeBase:: addChild(SyntaxTreeNodeBase* pChild, int ind )  {
	if (pChild)   {
		if (ind >= 0)  {
			_child[ind] = pChild;
		}
		pChild->_childIndex = ind;
		int simbling = 0;
		for (auto p = pChild; p != nullptr; p = p->getNextNode())  {    //所有兄弟节点都要有父节点
			p->_pParent = this;
			simbling++;
		}
		pChild->_siblings = ((simbling == 0) ? 1 : simbling);     //大哥存有同等兄弟节点的个数,不是所有父节点孩子的个数
	}
}



ClassTreeNode* SyntaxTreeNodeBase::getCurCurClassNode()
{
	if (s_stackCurClassZone.size() == 0)  {
		return nullptr;
	}
	return s_stackCurClassZone.top();
}

void SyntaxTreeNodeBase::insertClassNode(ClassTreeNode* node)
{
	s_stackCurClassZone.push(node);
}

void SyntaxTreeNodeBase::quitClassZone()
{
	assert(s_stackCurClassZone.size() > 0);
	s_stackCurClassZone.pop();
}








SubroutineDecNode* SyntaxTreeNodeBase::getCurSubroutineNode()
{
	if (s_stackCurSubroutineZone.size() == 0)  {
		return nullptr;
	}
	return s_stackCurSubroutineZone.top();
}

void SyntaxTreeNodeBase::insertSubRoutineNode(SubroutineDecNode* node)
{
	s_stackCurSubroutineZone.push(node);
}

void SyntaxTreeNodeBase::quitSubRoutineZone()
{
	assert(s_stackCurSubroutineZone.size() > 0);
	s_stackCurSubroutineZone.pop();
}



SubroutineBodyNode* SyntaxTreeNodeBase::getCurSubroutineBodyNode()             //目前在哪个函数体中,用于非复合语句添加树,可以用于闭包
{
	if (s_stackCurSubroutineBodyZone.size() == 0)  {
		return nullptr;
	}
	SubroutineBodyNode* node = (SubroutineBodyNode*)s_stackCurSubroutineBodyZone.top();
	return node;
}

void SyntaxTreeNodeBase::insertSubRoutineBodyNode(SubroutineBodyNode* node)
{
	s_stackCurSubroutineBodyZone.push(node);
}

void SyntaxTreeNodeBase::quitSubRoutineBodyZone()  
{
	assert(s_stackCurSubroutineBodyZone.size() > 0);
	s_stackCurSubroutineBodyZone.pop();
}



bool SyntaxTreeNodeBase::isInCompoundBody(SyntaxTreeNodeBase* node)
{
	if (node && node->getParentNode())  {
		if (node->getParentNode()->getNodeKind() == SyntaxTreeNodeBase::IF_STATEMENT_K ||
			node->getParentNode()->getNodeKind() == SyntaxTreeNodeBase::WHILE_STATEMENT_K)  {
			if (node->getChildIndex() == 1)  {
				return true;
			}
		}
	}
	return false;
}

bool SyntaxTreeNodeBase::isInCompound(SyntaxTreeNodeBase* node)
{
	if (node)  {
		if (node->getNodeKind() == SyntaxTreeNodeBase::IF_STATEMENT_K ||
			node->getNodeKind() == SyntaxTreeNodeBase::WHILE_STATEMENT_K)  {
			return true;
		}
	}
	return false;
}






CompondStatement* SyntaxTreeNodeBase::getCurCompoundStatmentNode(int* pNum)     //目前在哪个if 或while体中,用于添加if while 的语句
{
	if (s_stackCurCompoundStatmentZone.size() == 0)  {
		if (pNum)  { 
			*pNum = 0; 
		}
		return nullptr;
	}
	if (pNum)  { 
		*pNum = s_stackCurCompoundStatmentZone.size(); 
	}
	return s_stackCurCompoundStatmentZone.top();
}

void SyntaxTreeNodeBase::insertCompoundStatmentNode(CompondStatement* node)  
{    //在if while 开始后进入
	s_stackCurCompoundStatmentZone.push(node);
}

void SyntaxTreeNodeBase::quitCompoundStatmentZone()  
{
	assert(s_stackCurCompoundStatmentZone.size() > 0);
	s_stackCurCompoundStatmentZone.pop();
}




string SubroutineDecNode::getName()
{
	auto pNode = _child[SubroutineFiled::Name];
	return pNode->getLexeme();
}

string SubroutineDecNode::getSignName()
{
	auto pNode = _child[SubroutineFiled::Sign];
	return pNode->getLexeme();
}

int SubroutineDecNode::getFuncLocalsNum()
{
	return getSubroutineBody()->getFuncLocalsNum();
}

SubroutineBodyNode* SubroutineDecNode::getSubroutineBody()
{
	return (SubroutineBodyNode*)_child[SubroutineFiled::Body];
}

string SubroutineDecNode::getRetType()
{
	return getChildByIndex(SubroutineFiled::Ret)->getLexeme();
} 

SyntaxTreeNodeBase* SubroutineDecNode::getFirstParam()
{
	return _child[SubroutineFiled::Params];
}

bool SubroutineDecNode::hasVarDecInParams(SyntaxTreeNodeBase* node)
{
	auto params = _child[SubroutineDecNode::Params];
	while (params)  {
		if (params->getChildByIndex(VarDecNode::VarDec_Name)->getLexeme() == node->getLexeme())  {
			return true;
		}
		params = params->getNextNode();
	}
	return false;
}






int SubroutineBodyNode::getFuncLocalsNum()
{
	if (!getChildByIndex(BaseBlockBody::VarDec))  {   //没有变量声明
		return 0;
	}
	int nlocals = 0;
	/*for (auto q = getChildByIndex(SubroutineBody::VarDec); q != nullptr; q = q->getNextNode())  {
		for (auto p = q->getChildByIndex(VarDecNode::EVarDec::VarDec_Name); p != nullptr; p = p->getNextNode())  {
			nlocals++;
		}
	}*/
	for (auto q = getChildByIndex(BaseBlockBody::VarDec); q != nullptr; q = q->getNextNode())  {
		nlocals = nlocals + q->getChildByIndex(VarDecNode::EVarDec::VarDec_Name)->getSiblings();
	}
	return nlocals;
}

bool SubroutineBodyNode::hasVarDec(SyntaxTreeNodeBase* node)  {
	auto p = _varDecList.getHeadNode();
	if (!p)  {
		return true;
	}
	while (p)  {
		auto var_name = p->getChildByIndex(VarDecNode::VarDec_Name);
		for (; var_name != nullptr; var_name = var_name->getNextNode())  {
			if (var_name->getLexeme() == node->getLexeme())  {
				return true;
			}
		}
		p = p->getNextNode();
	}
	return false;
}

VarDecNode* SubroutineBodyNode::getCurVarDec()  {
	return (VarDecNode*)_varDecList.getCurNode();
}



SyntaxTreeNodeBase* AssignStatement::getChildByTag(string name)
{
	if (name == "var_name")  {
		return _child[AssignLetf];
	}
	else if (name == "var_rval")  {
		return _child[AssignRight];
	}
	return nullptr;
}

SyntaxTreeNodeBase* AssignStatement::getAssginLeft()
{
	return _child[AssignLetf];
}

SyntaxTreeNodeBase* AssignStatement::getAssginRight()
{
	return _child[AssignRight];
}




SyntaxTreeNodeBase* VarDecNode::getVarDecType()
{
	return _child[VarDecNode::VarDec_Type];
}

SyntaxTreeNodeBase* VarDecNode::getVarDecName()
{
	return _child[VarDecNode::VarDec_Name];
}











void TreeNodeList::Push(TreeNode* node)  
{     
	if (node != nullptr)  {
		if (_head == nullptr)  {
			TreeNode* curNode = getCurNode(node);
			if (curNode != node)  {  //要加入的节点是个链节点则要拆散一个一个的加
				_head = node;
				_cur = curNode;
			}
			else  {
				_head = _cur = node;
			}
		}
		else  {
			TreeNode* curNode = getCurNode(node);  //节点的当前节点,即最后一个节点
			if (curNode != node)  {                //要加入的节点是个链节点则要拆散一个一个的加
				_cur->setNextNode(node);
				_cur = curNode;
			}
			else  {
				_cur->setNextNode(node);
				_cur = node;
			}
		}
	}
}

TreeNode* TreeNodeList::getCurNode(TreeNode* node)  
{
	TreeNode* curNode = nullptr;
	while (node)  {
		curNode = node;
		node = node->getNextNode();
	}
	return curNode;
}

TreeNode* TreeNodeList::joinBy(TreeNodeList* node2)
{
	if (_cur)  {
		if (node2)  {
			_cur->setNextNode(node2->getHeadNode());
		}
	}
	else {
		if (node2)  {
			return node2->getHeadNode();
		}
	}
	return _head;
}