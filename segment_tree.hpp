#ifndef __SEGMENTTREE_H
#define __SEGMENTTREE_H


#include <vector>
#include <set>
#include <algorithm>

using std::vector;
using std::set;


template <class TKey, class TValue>
class TSegmentTree {
public:
	struct TKeyValue {
		TKey Start, End;
		TValue Value;
		TKeyValue(const TKey& start, const TKey& end, const TKey& value) :
			Start(start),
			End(end),
			Value(value) {
		}
	};

	TSegmentTree (const vector<TKeyValue>& data) {
		BuildTree(data);
	}

	int Search(const TKey& point, void (*callback) (const TValue&)) const {
		const TKey& minValue = Tree[TREE_ROOT].Start;
		const TKey& maxValue = Tree[TREE_ROOT].End;
		if (point < minValue || point >= maxValue) {
			return 0;
		}
		return FindContainingIntervals(point, TREE_ROOT, callback);
	}
	vector<TValue> Search(const TKey& point) const {
		const TKey& minValue = Tree[TREE_ROOT].Start;
		const TKey& maxValue = Tree[TREE_ROOT].End;
		if (point < minValue || point >= maxValue) {
			return vector<TValue>();
		}
		CollectorVector collector;
		FindContainingIntervals(point, TREE_ROOT, &collector);
		return collector.Results;
	}

	// there will be repeats in results, to avoid repeats use the next method
	void Search(const TKey& start, const TKey& end, void (* callback) (const TValue&)) const {
		const TKey& minValue = Tree[TREE_ROOT].Start;
		const TKey& maxValue = Tree[TREE_ROOT].End;
		if (end <= minValue || start >= maxValue) {
			return 0;
		}
		FindOverlappingIntervals(start, end, TREE_ROOT, callback);
	}
	//no repeats
	vector<TValue> Search(const TKey& start, const TKey& end) const {
		const TKey& minValue = Tree[TREE_ROOT].Start;
		const TKey& maxValue = Tree[TREE_ROOT].End;
		if (end <= minValue || start >= maxValue) {
			return 0;
		}
		CollectorSet collector;
		FindOverlappingIntervals(start, end, TREE_ROOT, &collector);
		return vector<TValue>(collector.Results.begin(), collector.Results.end());
	}


private:
	struct TNode {
		TKey Start, End;
		vector<TValue> Values;
		TNode(const TKey& start, const TKey& end) :
				Start(start),
				End(end) {
		}
		TNode() {
		}
	};
	static const int TREE_ROOT = 1;
	vector<TNode> Tree;

	void BuildTree(const vector<TKeyValue>& data) {
		vector<TKey> borders;
		for (int intervalIndex = 0; intervalIndex < data.size(); ++intervalIndex) {
			borders.push_back(data[intervalIndex].Start);
			borders.push_back(data[intervalIndex].End);
		}
		std::sort(borders.begin(), borders.end());
		const TKey maxValue = *borders.rbegin();
		vector<TNode> leafNodes;
		for (int nodeIndex = 0; nodeIndex < borders.size(); ++nodeIndex) {
			if (nodeIndex > 0 && borders[nodeIndex] == borders[nodeIndex - 1]) {
				continue;
			}
			const TKey& start = borders[nodeIndex];
			if (leafNodes.size()) {
				leafNodes.rbegin()->End = start;
			}
			leafNodes.push_back(TNode(start, start));
		}
		{// create tree structure
			int insertBefore = 1;
			int layerSize = 1;
			while (layerSize < leafNodes.size()) {
				insertBefore += layerSize;
				layerSize = layerSize << 1;
			}
			Tree = vector<TNode>(insertBefore, TNode(maxValue, maxValue));
			Tree.insert(Tree.end(), leafNodes.begin(), leafNodes.end());
		}
		//fill borders
		for (int position = Tree.size() - leafNodes.size() - 1; position >= TREE_ROOT; --position) {
			const int left_child = position << 1;
			const int right_child = (position << 1) + 1;
			if (left_child < Tree.size()) {
				Tree[position].Start = Tree[left_child].Start;
				Tree[position].End = Tree[left_child].End;
			}
			if (right_child < Tree.size()) {
				Tree[position].End = Tree[right_child].End;
			}
		}
		//fill values
		for (int intervalIndex = 0; intervalIndex < data.size(); ++intervalIndex) {
			const TKey& start = data[intervalIndex].Start;
			const TKey& end = data[intervalIndex].End;
			const TValue& value = data[intervalIndex].Value;
			TSegmentTree::PutInterval(start, end, value, TREE_ROOT);
		}
	}

	// results will be with repeats!!!!
	void FindOverlappingIntervals(const TKey& start, const TKey& end, const int startNode, void (* callback) (const TValue&)) const {
		if (!Tree[startNode].Values.empty()) {
			for (typename vector<TValue>::const_iterator valueIt = Tree[startNode].Values.begin();
					valueIt != Tree[startNode].Values.end(); ++valueIt) {
				callback(*valueIt);
			}
		}
		{
			const int leftChild = startNode << 1;
			if (leftChild < Tree.size() && Tree[leftChild].Start <= end && Tree[leftChild].End > start) {
				FindOverlappingIntervals(start, end, leftChild, callback);
			}
		}
		{
			const int rightChild = (startNode << 1) + 1;
			if (rightChild < Tree.size() && Tree[rightChild].Start <= end && Tree[rightChild].End > start) {
				FindOverlappingIntervals(start, end, rightChild, callback);
			}
		}
	}

	int FindContainingIntervals(const TKey& point, const int startNode, void (* callback) (const TValue&)) const {
		int hitsCount = Tree[startNode].Values.size();
		if (!Tree[startNode].Values.empty()) {
			for (typename vector<TValue>::const_iterator valueIt = Tree[startNode].Values.begin();
					valueIt != Tree[startNode].Values.end(); ++valueIt) {
				callback(*valueIt);
			}
		}
		{
			const int leftChild = startNode << 1;
			if (leftChild < Tree.size() && Tree[leftChild].Start <= point && Tree[leftChild].End > point) {
				hitsCount += FindContainingIntervals(point, leftChild, callback);
			}
		}
		{
			const int rightChild = (startNode << 1) + 1;
			if (rightChild < Tree.size() && Tree[rightChild].Start <= point && Tree[rightChild].End > point) {
				hitsCount += FindContainingIntervals(point, rightChild, callback);
			}
		}
		return hitsCount;
	}

	void PutInterval(const TKey& start, const TKey& end, const TValue& value, const int startNode) {
		const TKey& nodeStart = Tree.at(startNode).Start;
		const TKey& nodeEnd = Tree.at(startNode).End;
		if (nodeStart == start && nodeEnd == end) {
			Tree.at(startNode).Values.push_back(value);
			return;
		}
		const int leftChild = startNode << 1;
		const int rightChild = leftChild + 1;
		const TKey& leftChildEnd = Tree.at(leftChild).End;
		if (start < leftChildEnd) {
			TSegmentTree::PutInterval(start, std::min(leftChildEnd, end), value, leftChild);
		}
		if (end > leftChildEnd) {
			TSegmentTree::PutInterval(std::max(start, leftChildEnd), end, value, rightChild);
		}
	}

	struct CollectorVector {
		vector<TValue> Results;
		void operator()(TValue& value) {
			Results.push_back(value);
		}
	};
	struct CollectorSet {
		set<TValue> Results;
		void operator()(TValue& value) {
			Results.insert(value);
		}
	};

};


#endif
