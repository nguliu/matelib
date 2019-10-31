// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_BASE_TIMESTAMP_H
#define MATELIB_BASE_TIMESTAMP_H

#include "copyable.h"
#include <string>
#include <stdint.h>
#include <time.h>

namespace lfp
{
    class Timestamp : public copyable
    {
	public:
		Timestamp()
		  : microSecondsSinceEpoch_(0)
		{
		}

		explicit Timestamp(int64_t microSecondsSinceEpochArg)
			: microSecondsSinceEpoch_(microSecondsSinceEpochArg)
		{
		}

		void swap(Timestamp& that)
		{
			std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
		}

		std::string toString() const;

		/*以下返回当前时间的标准格式*/
		std::string toFormattedString(bool showMicroseconds = true) const;

		bool valid() const { return microSecondsSinceEpoch_ > 0; }

		int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
		time_t secondsSinceEpoch() const {
			return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
		}

		/*得到当前的UTC时间戳对象*/
		static Timestamp now();

		static Timestamp invalid()
		{
			return Timestamp();
		}

		//微秒到秒的转换进制
		static const int kMicroSecondsPerSecond = 1000 * 1000;

	private:
		int64_t microSecondsSinceEpoch_;	//保存从1900.1.1 0:0:0起经过的微秒数
    };


    inline bool operator<(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
    }

    inline bool operator==(Timestamp lhs, Timestamp rhs)
    {
        return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
    }

	//返回 high - low，单位为秒
	inline double timeDifference(Timestamp high, Timestamp low)
	{
		int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
		return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
	}
    
    inline Timestamp addTime(Timestamp timestamp, double seconds)
    {
        int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
        return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
    }

} //namespace lfp

#endif  // !MATELIB_BASE_TIMESTAMP_H
