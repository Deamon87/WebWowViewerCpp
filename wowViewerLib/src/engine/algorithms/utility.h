#ifndef UTILITY_STL_H
#define UTILITY_STL_H

namespace misc
{
	const std::size_t minval_radix = 7;
	const std::size_t maxval_radix = 8;

	struct partitioner
	{
		typedef std::pair<std::size_t, std::size_t> partition_range;

	public:
		explicit partitioner(const partition_range& range, const std::size_t count)
			: _left(std::size_t(0)), position(std::size_t(0)), _range(range), partitions(count)
		{
			length = static_cast<std::size_t>(
				std::ceil(this->distance() / (double)this->count_base()));
		}

		partitioner(const partitioner& p)
			: _left(p.left_base()), _range(p.range_base()), partitions(p.count_base()) { }

	public:
		const partitioner& operator[](std::size_t index) {
			position = index; return *this;
		}

	public:
		std::size_t size() const {
			return static_cast<std::size_t>(
				std::ceil(this->distance() / (double)this->size_base()));
		}

		std::size_t partition_size() const {
			return this->size_base();
		}

		std::size_t first() const {
			std::size_t pos = this->position_base() * this->size_base();
			return (pos <= _range.second) ? pos : _range.second;
		}
		std::size_t second() const {
			std::size_t pos = (this->position_base() + 1) * this->size_base();
			return (pos <= _range.second) ? pos : _range.second;
		}

	public:
		std::size_t distance() const {
			return this->range_base().second - this->range_base().first + 1;
		}

	public:
		std::size_t left_base() const { return _left; }
		partition_range range_base() const { return _range; }
		std::size_t count_base() const { return partitions; }
		std::size_t size_base() const { return length; }
		std::size_t position_base() const { return position; }

	protected:

			std::size_t _left;
			partition_range _range;
			std::size_t partitions;
			std::size_t length;
			std::size_t position;

	};



	template<class BidirIt, class _Pred>
	std::size_t sorted(BidirIt _First, BidirIt _Last, \
		std::size_t& position, _Pred compare)
	{
		bool is_sorted = true;

		BidirIt _MidIt = _First + (_Last - _First) / 2;
		if (compare(*_MidIt, *_First) || compare(*(_Last - 1), *_MidIt) ||
			compare(*(_Last - 1), *_First)) return !is_sorted;

		for (auto _FwdIt = _First; _FwdIt != _Last - 1 && is_sorted; _FwdIt++)
		{
			if (compare(*(_FwdIt + 1), *_FwdIt))
			{
				if (is_sorted == true)
					position = std::distance(_First, _FwdIt);

				is_sorted = false;
			}
		}

		return is_sorted;
	}

	template<class RandomIt>
	void print_out(RandomIt _First, RandomIt _Second)
	{
		for (auto it = _First; it != _Second; it++)
			std::cout << *it << " ";

		std::cout << "\n";
	}
}

#endif // UTILITY_STL_H
