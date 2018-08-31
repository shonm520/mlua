#include "Scanner.h"
#include "GramTreeNode.h"
#include "Visitor.h"



SyntaxTreeNodeBase::SyntaxTreeNodeBase(int nK) 
	:_pNext(nullptr), _pParent(nullptr),
	_childIndex(-1), _siblings(0)
{
	_nodeKind = (NodeKind)nK;
	memset(&_child, 0, sizeof(SyntaxTreeNodeBase*)* Child_Num_Const);
}

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

SyntaxTreeNodeBase* SyntaxTreeNodeBase::clone() 
{
	auto node = new SyntaxTreeNodeBase;
	node->_nodeKind = _nodeKind;
	node->_token = _token;
	node->_pParent = _pParent;
	node->_childIndex = _childIndex;
	node->_siblings = _siblings;
	return  node;
}


void SyntaxTreeNodeBase::clear(bool next)
{
	for (int i = 0; i < Child_Num_Const; i++)  {
		if (_child[i])  {
			_child[i]->clear(true);
		}
	}
	if (_pNext && next)  {
		SyntaxTreeNodeBase* p = _pNext;
		std::list<SyntaxTreeNodeBase*> listNode;
		while (p)  {
			listNode.push_back(p);
			p = p->getNextNode();
		}
		for (auto it = listNode.begin(); it != listNode.end(); ++it)  {
			(*it)->clear(false);
		}
	}
	delete this;
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





Value* Terminator::getVal()  
{
	if (!_val)  {
		if (_type == TERM_NUMBER)  {
			double num = strtod(_token.lexeme.c_str(), 0);
			_val = new Number(num);
		}
		else  if (_type == TERM_TRUE){
			_val = new BoolValue(true);
		}
		else  if (_type == TERM_FALSE){
			_val = new BoolValue(false);
		}
		else if (_type == TERM_STRING)  {
			_val = new String(_token.lexeme);
		}
		else  if (_type == TERM_NIL){
			_val = new Nil();
		}
	}
	return _val;
}




Value* IdentifierNode::getVal()  {
	if (!_val)  {
		_val = new String(_token.lexeme.c_str());
	}
	return _val;
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