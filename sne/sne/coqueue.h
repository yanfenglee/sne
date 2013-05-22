#pragma once

/**
 * ˫�̰߳�ȫ��queue, һ���߳�enqueue, һ���߳�dequeue, �ص�������������                     
 * �����߳�ͬʱ����ͬʱд��to do����������
 */
template<class data_t> class coqueue
{
public:
	coqueue()
	{
		node_t* node = new node_t;
		node->next = 0;
		m_head = m_tail = node;
	}
	~coqueue()
	{
		_clear();
		delete m_head;
	}

	void enqueue(const data_t& data)
	{
		node_t* node = new node_t;
		node->data = data;
		node->next = 0;
		//to do: tail_lock
		m_tail->next = node;
		m_tail = node;
		//to do: tail_unlock
	}
	bool dequeue(data_t& data)
	{
		//to do: head_lock
		node_t* node = m_head;
		node_t* new_head = node->next;
		if (new_head == 0)
		{
			//to do: head_unlock
			return false;
		}
		data = new_head->data;
		m_head = new_head;
		//to do: head_unlock
		delete node;
		return true;
	}

	bool empty()
	{
		return m_head->next == 0;
	}

private:
	struct node_t
	{
		data_t data;
		node_t* next;
	};	
	
	void _clear()
	{
		data_t data;
		while(dequeue(data));
	}
private:
	node_t* m_head;
	node_t* m_tail;
}; 