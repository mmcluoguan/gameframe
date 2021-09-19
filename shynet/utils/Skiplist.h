#pragma once
#include <jemalloc/jemalloc.h>
#include <algorithm>
#include <iostream>
#include <vector>

namespace shynet {
	namespace utils {

		//
		// 跳表
		//
		template <typename Key, typename Score>
		class SkipList : public Nocopy {
		public:
			using value_type = std::pair<Key, Score>;

			SkipList();
			virtual ~SkipList();

		public:
			// inserts elements
			void insert(const value_type& value);

			// erase element
			bool erase(const value_type& value);

			// access the last element
			void back(Key& key) const;

			// access the first element
			bool front(Key& key) const;

			// get rank by score and key
			uint32_t rank(const value_type& value) const;

			// get key by rank
			bool element(uint32_t rank, Key& key) const;
			bool element(uint32_t rank, value_type& value) const;

			// get ranklist
			// index    - from rank, >= 1
			// count    - get ranklist size
			int32_t ranklist(uint32_t index, uint32_t count, std::vector<Key>& list) const;
			int32_t ranklist(uint32_t index, uint32_t count, std::vector<value_type>& list) const;

			// get score range [min, max]
			int32_t range(const Score& min, const Score& max,
				uint32_t count, std::vector<Key>& list, const std::vector<Key>& excepts = std::vector<Key>()) const;
			int32_t range(const Score& min, const Score& max,
				uint32_t count, std::vector<value_type>& list, const std::vector<Key>& excepts = std::vector<Key>()) const;

			// clear
			void clear();

			// returns the number of elements
			uint32_t size() const { return m_Size; }

			// print for debug
			void print(int32_t width = 4);

		private:
			struct Node;

			enum {
				eMaxLevel = 32,
			};

		private:
			// 随机层数
			int32_t randomLevel();

			// 删除节点
			void eraseNode(Node* node, Node** update);

		private:
			struct Node* m_Head;
			struct Node* m_Tail;
			uint32_t            m_Size;
			int32_t             m_Level;
		};

		template <typename Key, typename Score>
		struct SkipList<Key, Score>::Node {
			struct Level {
				Node* forward;
				uint32_t    span;
			};

			Node()
				: backward(nullptr) {
			}

			Node(const value_type& value)
				: key(value.first),
				score(value.second),
				backward(nullptr) {
			}

			uint32_t span(int32_t l) const {
				return levels[l].span;
			}

			Node* forward(int32_t l) const {
				return levels[l].forward;
			}

			Key             key;
			Score           score;
			struct Node* backward;
			struct Level    levels[];
		};

		template <typename Key, typename Score>
		SkipList<Key, Score>::SkipList()
			: m_Tail(nullptr),
			m_Size(0),
			m_Level(1) {
			m_Head = new (je_malloc(sizeof(Node) + eMaxLevel * sizeof(typename Node::Level))) Node();

			for (uint32_t i = 0; i < eMaxLevel; ++i) {
				m_Head->levels[i].span = 0;
				m_Head->levels[i].forward = nullptr;
			}
		}

		template <typename Key, typename Score>
		SkipList<Key, Score>::~SkipList() {
			Node* next = nullptr;
			Node* node = m_Head->levels[0].forward;

			je_free(m_Head);

			while (node) {
				next = node->levels[0].forward;
				je_free(node);
				node = next;
			}
		}

		template <typename Key, typename Score>
		void SkipList<Key, Score>::insert(const value_type& value) {
			int32_t level;
			int32_t rank[eMaxLevel];

			Node* x = m_Head;
			Node* update[eMaxLevel];

			// span 某一层两个相连节点间间隔距离
			// rank记录的是经过路径(每一层)的最后一个节点是第几个
			// update记录的是 经过路径(每一层)的最后一个节点的前驱点
			for (int32_t i = m_Level - 1; i >= 0; --i) {
				rank[i] = i == (m_Level - 1) ? 0 : rank[i + 1];
				while (x->forward(i)
					&& (x->forward(i)->score > value.second
						|| (x->forward(i)->key < value.first
							&& x->forward(i)->score == value.second))) {
					rank[i] += x->span(i);
					x = x->forward(i);
				}

				// 当前层需要更新的节点
				update[i] = x;
			}

			// 随机层
			level = randomLevel();

			//
			if (level > m_Level) {
				for (int32_t i = m_Level; i < level; ++i) {
					rank[i] = 0;
					update[i] = m_Head;
					update[i]->levels[i].span = m_Size;
				}

				m_Level = level;
			}

			x = new (je_malloc(sizeof(Node) + level * sizeof(typename Node::Level))) Node(value);
			for (int32_t i = 0; i < level; ++i) {
				x->levels[i].forward = update[i]->levels[i].forward;
				update[i]->levels[i].forward = x;

				/* update span covered by update[i] as x is inserted here */
				x->levels[i].span = update[i]->levels[i].span - (rank[0] - rank[i]);
				update[i]->levels[i].span = (rank[0] - rank[i]) + 1;
			}

			/* increment span for untouched levels */
			for (int32_t i = level; i < m_Level; ++i) {
				++update[i]->levels[i].span;
			}

			x->backward = (update[0] == m_Head) ? nullptr : update[0];

			if (x->levels[0].forward) {
				x->levels[0].forward->backward = x;
			}
			else {
				m_Tail = x;
			}

			++m_Size;
		}

		template<typename Key, typename Score>
		bool SkipList<Key, Score>::erase(const value_type& value) {
			Node* x = m_Head;
			Node* update[eMaxLevel];

			for (int32_t i = m_Level - 1; i >= 0; --i) {
				while (x->forward(i)
					&& (x->forward(i)->score > value.second
						|| (x->forward(i)->key < value.first
							&& x->forward(i)->score == value.second))) {
					x = x->forward(i);
				}

				update[i] = x;
			}

			x = x->levels[0].forward;

			if (x && x->key == value.first) {
				eraseNode(x, update);
				je_free(x);

				return true;
			}

			return false;
		}

		template<typename Key, typename Score>
		bool SkipList<Key, Score>::front(Key& key) const {
			Node* x = m_Head;

			if (x->levels[0].forward) {
				x = x->levels[0].forward;
				key = x->key;

				return true;
			}

			return false;
		}

		template<typename Key, typename Score>
		void SkipList<Key, Score>::back(Key& key) const {
			Node* x = m_Tail;

			if (x) {
				key = x->key;
			}
		}

		template<typename Key, typename Score>
		uint32_t SkipList<Key, Score>::rank(const value_type& value) const {
			Node* x = m_Head;
			uint32_t rank = 0;

			for (int32_t i = m_Level - 1; i >= 0; --i) {
				while (x->forward(i)
					&& (x->forward(i)->score > value.second
						|| (x->forward(i)->key <= value.first
							&& x->forward(i)->score == value.second))) {
					rank += x->span(i);
					x = x->forward(i);
				}

				if (x != m_Head && x->key == value.first) {
					return rank;
				}
			}

			return 0;
		}

		template<typename Key, typename Score>
		bool SkipList<Key, Score>::element(uint32_t rank, Key& key) const {
			value_type value;

			if (element(rank, value)) {
				key = value.first;
				return true;
			}

			return false;
		}

		template<typename Key, typename Score>
		bool SkipList<Key, Score>::element(uint32_t rank, value_type& value) const {
			Node* x = m_Head;
			uint32_t traversed = 0;

			for (int32_t i = m_Level - 1; i >= 0; --i) {
				while (x->forward(i)
					&& traversed + x->span(i) < rank) {
					traversed += x->span(i);
					x = x->forward(i);
				}
			}

			++traversed;
			x = x->forward(0);

			if (x != nullptr) {
				value = value_type(x->key, x->score);
			}

			return x != nullptr;
		}

		template<typename Key, typename Score>
		int32_t SkipList<Key, Score>::ranklist(uint32_t index, uint32_t count, std::vector<Key>& list) const {
			typename std::vector<value_type> rangelist;
			int32_t nget = ranklist(index, count, rangelist);

			for (size_t i = 0; i < rangelist.size(); ++i) {
				list.push_back(rangelist[i].first);
			}

			return nget;
		}

		template<typename Key, typename Score>
		int32_t SkipList<Key, Score>::ranklist(uint32_t index, uint32_t count, std::vector<value_type>& list) const {
			Node* x = m_Head;
			int32_t nget = 0;
			uint32_t traversed = 0;

			index = std::max(index, (uint32_t)1);

			for (int32_t i = m_Level - 1; i >= 0; --i) {
				while (x->forward(i)
					&& traversed + x->span(i) < index) {
					traversed += x->span(i);
					x = x->forward(i);
				}
			}

			++traversed;
			x = x->forward(0);

			while (x && traversed <= index + count - 1) {
				Node* next = x->forward(0);

				++nget;
				++traversed;
				list.push_back(value_type(x->key, x->score));

				x = next;
			}

			return nget;
		}

		template<typename Key, typename Score>
		int32_t SkipList<Key, Score>::range(const Score& min, const Score& max, uint32_t count, std::vector<Key>& list, const std::vector<Key>& excepts) const {
			typename std::vector<value_type> rangelist;
			int32_t nget = range(min, max, count, rangelist, excepts);

			for (size_t i = 0; i < rangelist.size(); ++i) {
				list.push_back(rangelist[i].first);
			}

			return nget;
		}

		template<typename Key, typename Score>
		int32_t SkipList<Key, Score>::range(const Score& min, const Score& max, uint32_t count, std::vector<value_type>& list, const std::vector<Key>& excepts) const {
			int32_t nget = 0;
			Node* x = m_Head;

			if (min > max) {
				return 0;
			}

			if (m_Tail == nullptr || m_Tail->score > max) {
				return 0;
			}
			if (m_Head->forward(0) == nullptr
				|| m_Head->forward(0)->score < min) {
				return 0;
			}

			for (int32_t i = m_Level - 1; i >= 0; --i) {
				while (x->forward(i)
					&& x->forward(i)->score > max) {
					x = x->forward(i);
				}
			}

			x = x->forward(0);

			while (x
				&& list.size() < count
				&& (x->score > min || x->score == min)) {
				if (excepts.end()
					== std::find(excepts.begin(), excepts.end(), x->key)) {
					++nget;
					list.push_back(value_type(x->key, x->score));
				}

				x = x->forward(0);
			}

			return nget;
		}

		template<typename Key, typename Score>
		void SkipList<Key, Score>::clear() {
			Node* next = nullptr;
			Node* node = m_Head->forward(0);

			while (node) {
				next = node->forward(0);
				je_free(node);
				node = next;
			}

			// 初始化head
			for (int32_t i = 0; i < eMaxLevel; ++i) {
				m_Head->levels[i].span = 0;
				m_Head->levels[i].forward = nullptr;
			}

			m_Level = 1;
			m_Size = 0;
			m_Tail = nullptr;
		}

		template<typename Key, typename Score>
		void SkipList<Key, Score>::print(int32_t width) {
			Node* p = nullptr, * q = nullptr;

			std::cout << "level :" << m_Level << std::endl;

			//从最高层开始搜
			for (int32_t i = m_Level - 1; i >= 0; --i) {
				p = m_Head;

				while ((q = p->forward(i)) != nullptr) {
					for (int32_t j = 0; j < p->span(i) - 1; ++j) {
						for (int32_t k = 0; k < width; ++k) {
							std::cout << " ";
						}
					}

					std::cout << std::setw(width) << "(" << q->key << "," << q->score << ")";

					p = q;
				}

				std::cout << std::endl;
			}

			std::cout << std::endl;
		}

		template<typename Key, typename Score>
		void SkipList<Key, Score>::eraseNode(Node* node, Node** update) {
			for (int32_t i = 0; i < m_Level; ++i) {
				if (update[i]->forward(i) == node) {
					update[i]->levels[i].span += node->levels[i].span - 1;
					update[i]->levels[i].forward = node->levels[i].forward;
				}
				else {
					update[i]->levels[i].span -= 1;
				}
			}

			if (node->levels[0].forward) {
				node->levels[0].forward->backward = node->backward;
			}
			else {
				m_Tail = node->backward;
			}

			while (m_Level > 1 && m_Head->levels[m_Level - 1].forward == nullptr) {
				--m_Level;
			}

			--m_Size;
		}

		template<typename Key, typename Score>
		int32_t SkipList<Key, Score>::randomLevel() {
#define SKIPLIST_P  0.25

			int32_t level = 1;

			while (double(random() & 0xFFFF) < (SKIPLIST_P * 0xFFFF)) {
				level += 1;
			}

			return level < eMaxLevel ? level : eMaxLevel;
		}

	}
}
