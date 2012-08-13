#pragma once

#include <boost/function.hpp>

#include <Poco/SharedPtr.h>

#include <DB/Core/Field.h>
#include <DB/IO/ReadBuffer.h>
#include <DB/IO/WriteBuffer.h>
#include <DB/Columns/IColumn.h>


namespace DB
{

using Poco::SharedPtr;

/** Метаданные типа для хранения (столбца).
  * Содержит методы для сериализации/десериализации.
  */
class IDataType
{
public:
	/// Основное имя типа (например, UInt64).
	virtual std::string getName() const = 0;

	/// Является ли тип числовым.
	virtual bool isNumeric() const { return false; }

	/// Клонировать
	virtual SharedPtr<IDataType> clone() const = 0;

	/** Бинарная сериализация - для сохранения на диск / в сеть и т. п.
	  * Обратите внимание, что присутствует по два вида методов
	  * - для работы с единичными значениями и целыми столбцами.
	  */
	virtual void serializeBinary(const Field & field, WriteBuffer & ostr) const = 0;
	virtual void deserializeBinary(Field & field, ReadBuffer & istr) const = 0;

	/** Сериализация столбца.
	  * Можно передать callback, который будет вызван для некоторых значений.
	  *  callback вызывается для 0-го значения и возвращает индекс следующего значения,
	  *  для которого его следует вызвать.
	  * Это может быть использовано для одновременной записи индексного файла.
	  */
	typedef boost::function<size_t()> WriteCallback;
	virtual void serializeBinary(const IColumn & column, WriteBuffer & ostr,
		WriteCallback callback = WriteCallback()) const = 0;
	
	/** Считать не более limit значений. */
	virtual void deserializeBinary(IColumn & column, ReadBuffer & istr, size_t limit) const = 0;

	/** Текстовая сериализация - для вывода на экран / сохранения в текстовый файл и т. п.
	  * Без эскейпинга и квотирования.
	  */
	virtual void serializeText(const Field & field, WriteBuffer & ostr) const = 0;
	virtual void deserializeText(Field & field, ReadBuffer & istr) const = 0;

	/** Текстовая сериализация с эскейпингом, но без квотирования.
	  */
	virtual void serializeTextEscaped(const Field & field, WriteBuffer & ostr) const = 0;
	virtual void deserializeTextEscaped(Field & field, ReadBuffer & istr) const = 0;

	/** Текстовая сериализация в виде литерала, который может быть вставлен в запрос.
	  * Если compatible = true, то значение типа "массив" и "кортеж" ещё дополнительно записывается в кавычки,
	  *  чтобы текстовый дамп можно было загрузить в другую СУБД с этими значениями в виде строки.
	  */
	virtual void serializeTextQuoted(const Field & field, WriteBuffer & ostr, bool compatible = false) const = 0;
	virtual void deserializeTextQuoted(Field & field, ReadBuffer & istr, bool compatible = false) const = 0;

	/** Создать пустой столбец соответствующего типа.
	  */
	virtual SharedPtr<IColumn> createColumn() const = 0;

	/** Создать столбец соответствующего типа, содержащий константу со значением Field, длины size.
	  */
	virtual SharedPtr<IColumn> createConstColumn(size_t size, const Field & field) const = 0;

	/** Получить значение "по-умолчанию".
	  */
	virtual Field getDefault() const = 0;

	/// Вернуть приблизительный (оценочный) размер значения.
	virtual size_t getSizeOfField() const
	{
		throw Exception("getSizeOfField() method is not implemented for data type " + getName(), ErrorCodes::NOT_IMPLEMENTED);
	}

	virtual ~IDataType() {}
};


typedef Poco::SharedPtr<IDataType> DataTypePtr;
typedef std::vector<DataTypePtr> DataTypes;

}

