// @author: LiuFangPing
// @github: https://github.com/inmail
// @csdn: https://blog.csdn.net/qq_40843865

#ifndef MATELIB_RPC_SERIALIZER_H
#define MATELIB_RPC_SERIALIZER_H

#include <matelib/base/noncopyable.h>
#include <matelib/Buffer.h>
#include <algorithm>
#include <sstream>
#include <tuple>
#include <utility>

#include <cstdint>
#include <string.h>

namespace lfp
{
	//存储序列化数据的StreamBuffer类
	typedef lfp::Buffer StreamBuffer;

namespace detail
{	
	//序列化工具
	class Serializer : public noncopyable
	{
	public:
		enum ByteOrder {
			BigEndian,		//网络字节序
			LittleEndian	//主机字节序
		};

		Serializer()
		  : byteOrder_(LittleEndian),	//默认为主机字节序
			streamBuffer_()
		{ }
		//这里调用StreamBuffer的拷贝构造函数，会有一次内存拷贝
		Serializer(const StreamBuffer& buf, ByteOrder byteorder = LittleEndian)
		  : byteOrder_(byteorder),
			streamBuffer_(buf)
		{ }
		//这里调用StreamBuffer的移动拷贝构造函数，避免一次内存拷贝
		Serializer(StreamBuffer&& buf, ByteOrder byteorder = LittleEndian)
		  : byteOrder_(byteorder),
			streamBuffer_(std::forward<StreamBuffer>(buf))	//使用完美转发
		{ }

		void reset() {
			streamBuffer_.retrieveAll();
		}
		size_t readableBytes() const {
			return streamBuffer_.readableBytes();
		}
		void retrieve(size_t n) {	 //retrieve相关函数只是移动了游标，并没有真正删除数据
			streamBuffer_.retrieve(n);
		}
		void reverse2LittleEndian(char* data, size_t len) {
			if (byteOrder_ == BigEndian){
				std::reverse(data, data + len);
			}
		}
		void writeAndRetrieve(const char* data, size_t len) {
			streamBuffer_.append(data, len);
			streamBuffer_.retrieve(len);
		}
		const char* data() const {		//得到数据的起始位置
			return streamBuffer_.data();
		}
		const char* current() const {	//得到数据的当前位置
			return streamBuffer_.peek();
		}
		void clear() {		//真正的清空数据
			streamBuffer_.clear();
			reset();
		}

		StreamBuffer& streamBuffer() { return streamBuffer_; }


		// 直接给一个长度， 返回当前位置以后x个字节数据
		void getLenBytes(char* buf, size_t len) {
			::memmove(buf, current(), len);
			retrieve(len);
		}

		//从序列化数据中获取一个元素
		template<typename Tuple, std::size_t Id>
		void getv(Tuple& t) {
			(*this) >> std::get<Id>(t);	//获取元祖tuple的第Id个元素
		}

		//从序列化数据中获取一个元祖
		template<typename Tuple, std::size_t...I>
		Tuple getTuple(std::index_sequence<I...>) {
			Tuple t;
			std::initializer_list<int> { ((getv<Tuple, I>(t)), 0)... };
			return t;
			//以上是利用逗号运算符将其展开，详见 https://www.cnblogs.com/qicosmos/p/4325949.html#3634640
		}

		// 打包一个元祖
		template<typename Tuple, std::size_t...Is>
		void packageArgs(const Tuple& t, std::index_sequence<Is...>)
		{
			std::initializer_list<int> { ((*this << std::get<Is>(t)), 0)... };
		}

		template<typename...Args>
		void packageArgs(const std::tuple<Args...>& t)
		{
			packageArgs(t, std::index_sequence_for<Args...>{});
		}

		template<typename T>
		Serializer& operator>>(T& obj) {
			outputObj(obj); 
			return *this;
		}

		template<typename T>
		Serializer& operator<<(const T& obj) {
			inputObj(obj);
			return *this;
		}

	private:
		template<typename T>
		void outputObj(T& obj);

		template<typename T>
		void inputObj(T obj);

		ByteOrder byteOrder_;
		StreamBuffer streamBuffer_;
	};


	template<typename T>
	inline void Serializer::outputObj(T& obj)
	{
		const int len = sizeof(T);
		if (readableBytes() >= len)
		{
			char buf[len];
			::memcpy(buf, current(), len);
			retrieve(len);		//移动游标
			reverse2LittleEndian(buf, len);
			obj = *reinterpret_cast<T*>(buf);
		}
	}

	template<>
	inline void Serializer::outputObj(std::string& str)
	{
		const int marklen = sizeof(uint16_t);
		char buf[marklen];
		::memcpy(buf, current(), marklen);		//先获取string的长度
		retrieve(marklen);
		reverse2LittleEndian(buf, marklen);

		int len = *reinterpret_cast<uint16_t*>(buf);
		if (len == 0)
			return;

		str.insert(str.begin(), current(), current() + len);
		retrieve(len);
	}


	template<typename T>
	inline void Serializer::inputObj(T obj)
	{
		const int len = sizeof(T);
		char buf[len];
		const char* p = reinterpret_cast<const char*>(&obj);
		::memcpy(buf, p, len);
		reverse2LittleEndian(buf, len);

		streamBuffer_.append(buf, len);
	}

	template<>
	inline void Serializer::inputObj(const char* str)
	{
		// 先存入字符串长度
		uint16_t len = strlen(str);
		char* p = reinterpret_cast<char*>(&len);
		reverse2LittleEndian(p, sizeof(uint16_t));
		streamBuffer_.append(p, sizeof(uint16_t));

		// 存入字符串
		if (len == 0)
			return;

		streamBuffer_.append(str, len);
	}
	template<>
	inline void Serializer::inputObj(std::string str)	//无拷贝传递字符串
	{
		inputObj<const char*>(str.data());
	}

} // namespace detail



	template<typename T>
	struct type_xx{	typedef T type; };

	template<>
	struct type_xx<void>{ typedef int8_t type; };

	enum RpcStateCode {
		RPC_SUCCESS = 0,
		RPC_FUNCTION_NOTBIND,
		RPC_RECV_TIMEOUT
	};
	
	// wrap return value
	template<typename T>
	class value_t {
	public:
		typedef typename type_xx<T>::type valueType;

		value_t()
			: stateCode_(RPC_SUCCESS)
		{ }

		bool successful() { return stateCode_ == RPC_SUCCESS; }
		
		int stateCode() const { return stateCode_; }
		const std::string& message() const { return msg_; }
		const valueType& value() const { return value_; }

		void setStateCode(RpcStateCode code) { stateCode_ = code; }
		void setMessage(const std::string& msg) { msg_ = msg; }
		void setValue(const valueType& val) { value_ = val; }

		friend detail::Serializer& operator>>(detail::Serializer& in, value_t<T>& val) {
			in >> val.stateCode_ >> val.msg_;
			if (val.stateCode_ == RPC_SUCCESS) {
				in >> val.value_;
			}
			return in;
		}
		friend detail::Serializer& operator<<(detail::Serializer& out, const value_t<T>& val) {
			out << val.stateCode_ << val.msg_ << val.value_;
			return out;
		}

	private:
		RpcStateCode stateCode_;
		std::string msg_;
		valueType value_;
	};

} // namespace lfp

#endif // !MATELIB_RPC_SERIALIZER_H