#pragma once

#include "CvString.h"

// CvTaggedSaveFormatWrapper.h

#ifndef CIV4_TAGGED_SAVE_FORMAT_WRAPPER_H
#define CIV4_TAGGED_SAVE_FORMAT_WRAPPER_H

//	Forward declarations
class DictionaryEntry;

//	Class types we know how to remap on load (so that
//	if new classes of this type are added the load still ties up the old
//	instances correctly
typedef enum
{
	REMAPPED_CLASS_TYPE_NONE = -1,

	REMAPPED_CLASS_TYPE_BUILDINGS,
	REMAPPED_CLASS_TYPE_BUILDING_CLASSES,
	REMAPPED_CLASS_TYPE_UNITS,
	REMAPPED_CLASS_TYPE_UNIT_CLASSES,
	REMAPPED_CLASS_TYPE_BONUSES,
	REMAPPED_CLASS_TYPE_PROJECTS,
	REMAPPED_CLASS_TYPE_SPECIALISTS,
	REMAPPED_CLASS_TYPE_IMPROVEMENTS,
	REMAPPED_CLASS_TYPE_RELIGIONS,
	REMAPPED_CLASS_TYPE_COMBATINFOS,
	REMAPPED_CLASS_TYPE_PROMOTIONS,
	REMAPPED_CLASS_TYPE_CORPORATIONS,
	REMAPPED_CLASS_TYPE_TECHS,
	REMAPPED_CLASS_TYPE_CIVICS,
	REMAPPED_CLASS_TYPE_VOTES,
	REMAPPED_CLASS_TYPE_VOTE_SOURCES,
	REMAPPED_CLASS_TYPE_SPECIAL_UNITS,
	REMAPPED_CLASS_TYPE_SPECIAL_BUILDINGS,
	REMAPPED_CLASS_TYPE_UPKEEPS,
	REMAPPED_CLASS_TYPE_HURRIES,
	REMAPPED_CLASS_TYPE_FEATURES,
	REMAPPED_CLASS_TYPE_CIVIC_OPTIONS,
	REMAPPED_CLASS_TYPE_BUILDS,
	REMAPPED_CLASS_TYPE_TERRAINS,
	REMAPPED_CLASS_TYPE_ROUTES,
	REMAPPED_CLASS_TYPE_VICTORIES,
	REMAPPED_CLASS_TYPE_LEADERHEADS,
	REMAPPED_CLASS_TYPE_CIVILIZATIONS,
	REMAPPED_CLASS_TYPE_GAMEOPTIONS,
	REMAPPED_CLASS_TYPE_MPOPTIONS,
	REMAPPED_CLASS_TYPE_UNITAIS,
	REMAPPED_CLASS_TYPE_EVENTS,
	REMAPPED_CLASS_TYPE_EVENT_TRIGGERS,
	REMAPPED_CLASS_TYPE_GAMESPEEDS,
	REMAPPED_CLASS_TYPE_PROPERTIES,

	NUM_REMAPPED_TYPES
} RemappedClassType;

//	Save element types used in the save format
typedef enum
{
	SAVE_VALUE_TYPE_CHAR = 0,
	SAVE_VALUE_TYPE_BYTE,
	SAVE_VALUE_TYPE_CHAR_ARRAY,
	SAVE_VALUE_TYPE_BYTE_ARRAY,
	SAVE_VALUE_TYPE_BOOL,
	SAVE_VALUE_TYPE_BOOL_ARRAY,
	SAVE_VALUE_TYPE_SHORT,
	SAVE_VALUE_TYPE_UNSIGNED_SHORT,
	SAVE_VALUE_TYPE_SHORT_ARRAY,
	SAVE_VALUE_TYPE_UNSIGNED_SHORT_ARRAY,
	SAVE_VALUE_TYPE_INT,
	SAVE_VALUE_TYPE_UNSIGNED_INT,
	SAVE_VALUE_TYPE_INT_ARRAY,
	SAVE_VALUE_TYPE_UNSIGNED_INT_ARRAY,
	SAVE_VALUE_TYPE_LONG,
	SAVE_VALUE_TYPE_UNSIGNED_LONG,
	SAVE_VALUE_TYPE_LONG_ARRAY,
	SAVE_VALUE_TYPE_UNSIGNED_LONG_ARRAY,
	SAVE_VALUE_TYPE_FLOAT,
	SAVE_VALUE_TYPE_FLOAT_ARRAY,
	SAVE_VALUE_TYPE_DOUBLE,
	SAVE_VALUE_TYPE_DOUBLE_ARRAY,
	SAVE_VALUE_TYPE_WSTRING,
	SAVE_VALUE_TYPE_STRING,
	SAVE_VALUE_TYPE_STD_STRING,
	SAVE_VALUE_TYPE_STD_WSTRING,
	SAVE_VALUE_TYPE_WSTRING_ARRAY,
	SAVE_VALUE_TYPE_STRING_ARRAY,
	SAVE_VALUE_TYPE_CLASS_ENUM,
	SAVE_VALUE_TYPE_CLASS_INT_ARRAY,
	SAVE_VALUE_TYPE_CLASS_BOOL_ARRAY,
	SAVE_VALUE_TYPE_CLASS_ENUM_ARRAY,
} SaveValueType;

#define SAVE_VALUE_ANY	((SaveValueType)-1)

//	Indexed by their old enum ids arrays these structures contain the new enum id
//	and the (common and tying togther) type string
typedef struct EnumInfo
{
	int			m_id;	//	New id (-1 if not yet known)
	bool		m_lookedUp;
	CvString	m_szType;
} EnumInfo;

//
//	This class provides wrapping for elemnts that must be serialized in such a way
//	as to make them self-describing in terms of binding them to a provided name
//
class CvTaggedSaveFormatWrapper
{
public:
	//	Get the singleton instance
	static CvTaggedSaveFormatWrapper& getSaveFormatWrapper(void);

	void reset(bool useTaggedFormat);
	inline bool isUsingTaggedFormat()
	{
		return m_useTaggedFormat;
	}

	//	Call this to release memory and push any warnings to the UI after a load/save
	//	completes
	void		close(void);

	//	How many members of a given class type were present at save time?
	int			getNumClassEnumValues(RemappedClassType classType);
	//	Translate explicitly from an old enum value to the current
	int			getNewClassEnumValue(RemappedClassType classType, int oldValue, bool allowMissing = false);

	//	Attach the wrapper to a stream
	void		AttachToStream(FDataStreamBase* pStream);

	void		WriteStartObject(const char* name);
	void		WriteEndObject(void);

	//	The following methods must be used for entities that correspond to class enums
	void		WriteClassEnum(const char* name, RemappedClassType classType, int value);
	void		WriteClassEnum(const char* name, RemappedClassType classType, short value);
	//	The following are for arrays whose index is a class enum value
	void		WriteClassArray(const char* name, RemappedClassType classType, int count, const int values[]);
	void		WriteClassArray(const char* name, RemappedClassType classType, int count, const bool values[]);
	//	Array whose values are class enum entities
	void		WriteClassEnumArray(const char* name, RemappedClassType classType, int count, short values[]);
	void		WriteClassEnumArray(const char* name, RemappedClassType classType, int count, int values[]);
	//	The following are for arrays whose index is a class enum value and value is another class enum value
	void		WriteClassArrayOfClassEnum(const char* name, RemappedClassType indexClassType, RemappedClassType valueClassType, int count, const int values[]);

	//	The following methods are direct replacements for the write calls
	//	in the underlying FDataStreamBase 
	void		Write(const char* name, char value);
	void		Write(const char* name, byte value);
	void		Write(const char* name, int count, const  char values[]);
	void		Write(const char* name, int count, const  byte values[]);

	void		Write(const char* name, bool value);
	void		Write(const char* name, int count, const bool values[]);

	void		Write(const char* name, short value);
	void		Write(const char* name, unsigned short value);
	void		Write(const char* name, int count, const short values[]);
	void		Write(const char* name, int count, const unsigned short values[]) ;

	void		Write(const char* name, int value);
	void		Write(const char* name, unsigned int value) ;
	void 		Write(const char* name, int count, const int values[]);
	void		Write(const char* name, int count, const unsigned int values[]) ;

	void		Write(const char* name, long value);
	void		Write(const char* name, unsigned long  value) ;
	void 		Write(const char* name, int count, const long values[]);
	void		Write(const char* name, int count, const unsigned long values[]) ;

	void		Write(const char* name, float value);
	void		Write(const char* name, int count, const float values[]);

	void		Write(const char* name, double value);
	void		Write(const char* name, int count, const double values[]);
	
	void		WriteString(const char* name, const wchar *szName);
	void		WriteString(const char* name, const char *szName);
	void		WriteString(const char* name, const std::string& szName);
	void		WriteString(const char* name, const std::wstring& szName);
	void		WriteString(const char* name, int count, std::string values[]);
	void		WriteString(const char* name, int count, std::wstring values[]);
	
	void		ReadStartObject(const char* name);
	void		ReadEndObject(void);

	//	The following methods must be used for entities that correspond to class enums
	void		ReadClassEnum(const char* name, RemappedClassType classType, int* value, bool allowMissing = false);
	void		ReadClassEnum(const char* name, RemappedClassType classType, short* value, bool allowMissing = false);
	//	The following are for arrays whose index is a class enum value
	void		ReadClassArray(const char* name, RemappedClassType classType, int count, int values[], bool allowMissing = false, bool allowRawArray = false);
	void		ReadClassArray(const char* name, RemappedClassType classType, int count, bool values[], bool allowMissing = false, bool allowRawArray = false);
	//	Array whose values are class enum entities
	void		ReadClassEnumArray(const char* name, RemappedClassType classType, int count, short values[], bool allowMissing = false);
	void		ReadClassEnumArray(const char* name, RemappedClassType classType, int count, int values[], bool allowMissing = false);
	//	The following are for arrays whose index is a class enum value and value is another class enum value
	void		ReadClassArrayOfClassEnum(const char* name, RemappedClassType indexClassType, RemappedClassType valueClassType, int count, int values[]);

	//	The following methods are direct replacements for the read calls
	//	in the underlying FDataStreamBase 
	void		ReadString(const char* name, char **szName);
	void		ReadString(const char* name, wchar **szName);
	void		ReadString(const char* name, std::string& szName);
	void		ReadString(const char* name, std::wstring& szName);
	void		ReadString(const char* name, int count, std::string values[]);
	void		ReadString(const char* name, int count, std::wstring values[]);
	
	void		Read(const char* name, char *);
	void		Read(const char* name, byte *);
	void		Read(const char* name, int count, char values[]);
	void		Read(const char* name, int count, byte values[]);
	void		Read(const char* name, bool *);
	void		Read(const char* name, int count, bool values[], bool bAllowTruncation = false);
	void		Read(const char* name, short	*s);
	void		Read(const char* name, unsigned short	*s) ;
	void		Read(const char* name, int count, short values[]);
	void		Read(const char* name, int count, unsigned short values[]);
	void		Read(const char* name, int* i);
	void		Read(const char* name, unsigned int* i);
	void 		Read(const char* name, int count, int values[]);
	void 		Read(const char* name, int count, unsigned int values[]);

	void		Read(const char* name, long* l);
	void		Read(const char* name, unsigned long* l) ;
	void 		Read(const char* name, int count, long values[]);
	void 		Read(const char* name, int count, unsigned long values[]) ;

	void		Read(const char* name, float* value);
	void		Read(const char* name, int count, float values[]);

	void		Read(const char* name, double* value);
	void		Read(const char* name, int count, double values[]);

	void		SkipElement(const char* name, SaveValueType saveType);

	//	Methods to allow warnings and errors associated with the load/save to be reported
	void		warning(const char* msg);
	void		error(const char* msg);	//	Will terminate the app with a thrown exceptionm after displaying the message

private:
	CvTaggedSaveFormatWrapper();
	virtual ~CvTaggedSaveFormatWrapper();

	int			getId(const char* name, SaveValueType type, bool addToDictionary);
	void		WriteObjectDelimiter(const char* name, bool bStart);
	void		WriteClassMappingTable(RemappedClassType classType);
	void		WriteClassMappingTables(void);
	bool		Expect(const char* name, SaveValueType type);
	void		SkipElement(void);
	void		ConsumeBytes(int numBytes);
	void		ReadDictionaryElement(void);
	void		ReadClassMap(void);
	void		ReadObjectDelimiter(void);
	char*		NormalizeName(const char* name);
	void		RemoveArrayIndices(char* str, char* brace);
	void		HandleIncompatibleSave(const char* reason);
	void		HandleRecoverableIncompatibleSave(const char* reason);

public:
	bool		m_requestUseTaggedFormat;
private:
	FDataStreamBase*						m_stream;
	std::vector<DictionaryEntry>			m_idDictionary;
	std::map<CvString,DictionaryEntry>		m_reverseIdDictionary;
	bool									m_useTaggedFormat;
	bool									m_writtenMappingTables;
	int										m_nestingDepth;
	bool									m_bReadNextElementHeader;
	int										m_iNextElementType;
	int										m_iNextElementNameId;
	std::vector<EnumInfo>					m_enumMaps[NUM_REMAPPED_TYPES];
	CvString								m_delimiterName;
	bool									m_delimiterIsStart;
	int										m_streamNestingDepth;	//	On load of the read stream
	std::vector<CvWString>					m_warnings;
	bool									m_inUse;
};

#define WRAPPER_WRITE(wrapper,className,name)	(wrapper).Write(className "::" #name,name)
#define WRAPPER_WRITE_DECORATED(wrapper,className,name,saveName)	(wrapper).Write(className "::" saveName,name)
#define WRAPPER_WRITE_ARRAY(wrapper,className,count,name)	(wrapper).Write(className "::" #name,count,name)
#define WRAPPER_WRITE_ARRAY_DECORATED(wrapper,className,count,name,saveName)	(wrapper).Write(className "::" saveName,count,name)
#define WRAPPER_WRITE_STRING(wrapper,className,name)	(wrapper).WriteString(className "::" #name,name)
#define WRAPPER_WRITE_STRING_DECORATED(wrapper,className,name,saveName)	(wrapper).WriteString(className "::" saveName,name)
#define WRAPPER_WRITE_STRING_ARRAY(wrapper,className,count,name)	(wrapper).WriteString(className "::" #name,count,name)
#define WRAPPER_WRITE_CLASS_ENUM(wrapper,className,classType,name)	(wrapper).WriteClassEnum(className "::" #name,classType,name)
#define WRAPPER_WRITE_CLASS_ENUM_DECORATED(wrapper,className,classType,name,saveName)	(wrapper).WriteClassEnum(className "::" saveName,classType,name)
#define WRAPPER_WRITE_CLASS_ARRAY(wrapper,className,classType,count,name)	(wrapper).WriteClassArray(className "::" #name,classType,count,name)
#define WRAPPER_WRITE_CLASS_ARRAY_OF_CLASS_ENUM(wrapper,className,classType,valueClassType,count,name)	(wrapper).WriteClassArrayOfClassEnum(className "::" #name,classType,valueClassType,count,name)
#define WRAPPER_WRITE_CLASS_ENUM_ARRAY(wrapper,className,classType,count,name) (wrapper).WriteClassEnumArray(className "::" #name, classType,count,name)

#define WRAPPER_WRITE_OBJECT_START(wrapper)	(wrapper).WriteStartObject(__FUNCTION__)
#define WRAPPER_WRITE_OBJECT_END(wrapper)	(wrapper).WriteEndObject()

#define WRAPPER_READ(wrapper,className,name)	(wrapper).Read(className "::" #name,name)
#define WRAPPER_READ_DECORATED(wrapper,className,name,saveName)	(wrapper).Read(className "::" saveName,name)
#define WRAPPER_READ_ARRAY(wrapper,className,count,name)	(wrapper).Read(className "::" #name,count,name)
#define WRAPPER_READ_ARRAY_ALLOW_TRUNCATE(wrapper,className,count,name)	(wrapper).Read(className "::" #name,count,name,true)
#define WRAPPER_READ_ARRAY_DECORATED(wrapper,className,count,name,saveName)	(wrapper).Read(className "::" saveName,count,name)
#define WRAPPER_READ_STRING(wrapper,className,name)	(wrapper).ReadString(className "::" #name,name)
#define WRAPPER_READ_STRING_DECORATED(wrapper,className,name,saveName)	(wrapper).ReadString(className "::" saveName,name)
#define WRAPPER_READ_STRING_ARRAY(wrapper,className,count,name)	(wrapper).ReadString(className "::" #name,count,name)
#define WRAPPER_READ_CLASS_ENUM(wrapper,className,classType,name)	(wrapper).ReadClassEnum(className "::" #name,classType,name)
#define WRAPPER_READ_CLASS_ENUM_ALLOW_MISSING(wrapper,className,classType,name)	(wrapper).ReadClassEnum(className "::" #name,classType,name,true)
#define WRAPPER_READ_CLASS_ENUM_DECORATED(wrapper,className,classType,name,saveName)	(wrapper).ReadClassEnum(className "::" saveName,classType,name)
#define WRAPPER_READ_CLASS_ARRAY(wrapper,className,classType,count,name)	(wrapper).ReadClassArray(className "::" #name,classType,count,name)
#define WRAPPER_READ_CLASS_ARRAY_ALLOW_MISSING(wrapper,className,classType,count,name)	(wrapper).ReadClassArray(className "::" #name,classType,count,name,true)
#define WRAPPER_READ_CLASS_ARRAY_OF_CLASS_ENUM(wrapper,className,classType,valueClassType,count,name)	(wrapper).ReadClassArrayOfClassEnum(className "::" #name,classType,valueClassType,count,name)
#define WRAPPER_READ_CLASS_ENUM_ARRAY(wrapper,className,classType,count,name) (wrapper).ReadClassEnumArray(className "::" #name, classType,count,name)
#define WRAPPER_READ_CLASS_ENUM_ARRAY_ALLOW_MISSING(wrapper,className,classType,count,name) (wrapper).ReadClassEnumArray(className "::" #name, classType,count,name,true)

#define	WRAPPER_SKIP_ELEMENT(wrapper,className,name,type)	(wrapper).SkipElement(className "::" #name, type)

#define WRAPPER_READ_OBJECT_START(wrapper)	(wrapper).ReadStartObject(__FUNCTION__)
#define WRAPPER_READ_OBJECT_END(wrapper)	(wrapper).ReadEndObject()

//	Extra macros used only in order to support older saves wheer things that used to be simple values are now enums
#define WRAPPER_READ_OPTIONAL_CLASS_ARRAY(wrapper,className,classType,count,name)	(wrapper).ReadClassArray(className "::" #name,classType,count,name,true,true)

#endif

