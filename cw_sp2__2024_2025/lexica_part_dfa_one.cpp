#define _CRT_SECURE_NO_WARNINGS
/************************************************************
* N.Kozak // Lviv'2024-2025 // lexica_part_dfa__2025        *
*                         file: lexicapart_dfa_one.cpp      *
*                                                  (draft!) *
*************************************************************/
//#define MAX_DFA_SCAN_LEXEM_SIZE_FOR_TOKEN_PARSING 16
//#define USE_DFA_FOR_TOKEN_PARSING
//#define USE_DFA_TO_ACCEPT_KEYWORD
//#define USE_DFA_TO_ACCEPT_IDENTIFIER
//#define USE_DFA_TO_ACCEPT_UNSIGNEDVALUE

#define TOKENS_RE         ";|:=|=:|\\+|-|\\*|,|==|!=|:|\\[|\\]|\\(|\\)|\\{|\\}|<=|>=|[_0-9A-Za-z]+|[^ \t\r\f\v\n]"
#define KEYWORDS_RE       ";|:=|=:|\\+|-|\\*|,|==|!=|:|\\[|\\]|\\(|\\)|\\{|\\}|NAME|DATA|BODY|END|BREAK|CONTINUE|GET|PUT|IF|ELSE|FOR|TO|DOWNTO|DO|WHILE|REPEAT|UNTIL|GOTO|DIV|MOD|<=|>=|NOT|AND|OR|INTEGER16"
#define IDENTIFIERS_RE    "_[a-z][a-z][A-Z][A-Z]"
#define UNSIGNEDVALUES_RE "0|[1-9][0-9]*"

#include "dfa.hpp"
#include "matcher_by_dfa.hpp"
#include "file1.hpp"
#include "file2.hpp"
#include "file3.hpp"
#include "file4.hpp"

#define DEFAULT_INPUT_FILE "file5.k03"
//#define DEFAULT_INPUT_FILE "../other_test_programs_2025/file4.k03"

#define SUCCESS_STATE 0

#define DEBUG_MODE 4096

#define RERUN_MODE

#define VALUE_SIZE 4

#define MAX_TEXT_SIZE 8192
#define MAX_WORD_COUNT (MAX_TEXT_SIZE / 5)
#define MAX_LEXEM_SIZE 1024
#define MAX_VARIABLES_COUNT 256
#define MAX_KEYWORD_COUNT 64

#define KEYWORD_LEXEME_TYPE 1
#define IDENTIFIER_LEXEME_TYPE 2 // #define LABEL_LEXEME_TYPE 8
#define VALUE_LEXEME_TYPE 4
#define UNEXPEXTED_LEXEME_TYPE 127

#ifndef LEXEM_INFO_
#define LEXEM_INFO_
struct NonContainedLexemInfo;
struct LexemInfo {
public:
	char lexemStr[MAX_LEXEM_SIZE];
	unsigned long long int lexemId;
	unsigned long long int tokenType;
	unsigned long long int ifvalue;
	unsigned long long int row;
	unsigned long long int col;
	// TODO: ...

	LexemInfo();
	LexemInfo(const char* lexemStr, unsigned long long int lexemId, unsigned long long int tokenType, unsigned long long int ifvalue, unsigned long long int row, unsigned long long int col);
	LexemInfo(const NonContainedLexemInfo& nonContainedLexemInfo);
};
#endif

#ifndef NON_CONTAINED_LEXEM_INFO_
#define NON_CONTAINED_LEXEM_INFO_
struct LexemInfo;
struct NonContainedLexemInfo {
	//char lexemStr[MAX_LEXEM_SIZE]; 
	char* lexemStr;
	unsigned long long int lexemId;
	unsigned long long int tokenType;
	unsigned long long int ifvalue;
	unsigned long long int row;
	unsigned long long int col;
	// TODO: ...

	NonContainedLexemInfo();
	NonContainedLexemInfo(const LexemInfo& lexemInfo);
};
#endif

void printLexemes(struct LexemInfo* lexemInfoTable, char printBadLexeme/* = 0*/);
void printLexemesToFile(struct LexemInfo* lexemInfoTable, char printBadLexeme, const char* filename);
unsigned int getIdentifierId(char(*identifierIdsTable)[MAX_LEXEM_SIZE], char* str);
unsigned int tryToGetIdentifier(struct LexemInfo* lexemInfoInTable, char(*identifierIdsTable)[MAX_LEXEM_SIZE]);
unsigned int tryToGetUnsignedValue(struct LexemInfo* lexemInfoInTable);
int commentRemover(char* text, const char* openStrSpc/* = "//"*/, const char* closeStrSpc/* = "\n"*/);
void prepareKeyWordIdGetter(char* keywords_, char* keywords_re);
unsigned int getKeyWordId(char* keywords_, char* lexemStr, unsigned int baseId);
char tryToGetKeyWord(struct LexemInfo* lexemInfoInTable);
void setPositions(const char* text, struct LexemInfo* lexemInfoTable);
struct LexemInfo lexicalAnalyze(struct LexemInfo* lexemInfoInPtr, char(*identifierIdsTable)[MAX_LEXEM_SIZE]);
struct LexemInfo tokenize(char* text, struct LexemInfo** lastLexemInfoInTable, char(*identifierIdsTable)[MAX_LEXEM_SIZE], struct LexemInfo(*lexicalAnalyzeFunctionPtr)(struct LexemInfo*, char(*)[MAX_LEXEM_SIZE]));

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include <fstream>
#include <iostream>
//#include <algorithm>
#include <iterator>
#include <regex>

//struct LexemInfo {
//	char lexemStr[MAX_LEXEM_SIZE];
//	unsigned int lexemId;
//	unsigned int tokenType;
//	unsigned int ifvalue;
//	unsigned int row;
//	unsigned int col;
//	// TODO: ...
//};

#define MAX_ACCESSORY_STACK_SIZE_123 128

char tempStrFor_123[MAX_TEXT_SIZE/*?TODO:... MAX_ACCESSORY_STACK_SIZE_123 * 64*/] = { '\0' };
unsigned long long int tempStrForCurrIndex = 0;

struct LexemInfo lexemesInfoTable[MAX_WORD_COUNT];// = { { "", 0, 0, 0 } };
struct LexemInfo* lastLexemInfoInTable = lexemesInfoTable; // first for begin

char identifierIdsTable[MAX_WORD_COUNT][MAX_LEXEM_SIZE] = { "" };

LexemInfo::LexemInfo() {
	lexemStr[0] = '\0';
	lexemId = 0;
	tokenType = 0;
	ifvalue = 0;
	row = ~0;
	col = ~0;
}
LexemInfo::LexemInfo(const char* lexemStr, unsigned long long int lexemId, unsigned long long int tokenType, unsigned long long int ifvalue, unsigned long long int row, unsigned long long int col) {
	strncpy(this->lexemStr, lexemStr, MAX_LEXEM_SIZE);
	this->lexemId = lexemId;
	this->tokenType = tokenType;
	this->ifvalue = ifvalue;
	this->row = row;
	this->col = col;
}
LexemInfo::LexemInfo(const NonContainedLexemInfo& nonContainedLexemInfo) {
	strncpy(lexemStr, nonContainedLexemInfo.lexemStr, MAX_LEXEM_SIZE);
	lexemId = nonContainedLexemInfo.lexemId;
	tokenType = nonContainedLexemInfo.tokenType;
	ifvalue = nonContainedLexemInfo.ifvalue;
	row = nonContainedLexemInfo.row;
	col = nonContainedLexemInfo.col;
}

NonContainedLexemInfo::NonContainedLexemInfo() {
	(lexemStr = tempStrFor_123 + tempStrForCurrIndex)[0] = '\0';
	tempStrForCurrIndex += 32;// MAX_LEXEM_SIZE;
	lexemId = 0;
	tokenType = 0;
	ifvalue = 0;
	row = ~0;
	col = ~0;
}
NonContainedLexemInfo::NonContainedLexemInfo(const LexemInfo& lexemInfo) {
	//strncpy(lexemStr, lexemInfo.lexemStr, MAX_LEXEM_SIZE); // 
	lexemStr = (char*)lexemInfo.lexemStr;
	lexemId = lexemInfo.lexemId;
	tokenType = lexemInfo.tokenType;
	ifvalue = lexemInfo.ifvalue;
	row = lexemInfo.row;
	col = lexemInfo.col;
}

void printLexemes(struct LexemInfo* lexemInfoTable, char printBadLexeme) {
	if (printBadLexeme) {
		printf("Bad lexeme:\r\n");
	}
	else {
		printf("Lexemes table:\r\n");
	}
	printf("-------------------------------------------------------------------\r\n");
	//printf("index\t\tlexeme\t\tid\ttype\tifvalue\trow\tcol\r\n");
	printf("index           lexeme          id      type    ifvalue row     col\r\n");
	printf("-------------------------------------------------------------------\r\n");
	for (unsigned long long int index = 0; (!index || !printBadLexeme) && lexemInfoTable[index].lexemStr[0] != '\0'; ++index) {
		printf("%5llu%17s%12llu%10llu%11llu%4lld%8lld\r\n", index, lexemInfoTable[index].lexemStr, lexemInfoTable[index].lexemId, lexemInfoTable[index].tokenType, lexemInfoTable[index].ifvalue, lexemInfoTable[index].row, lexemInfoTable[index].col);
	}
	printf("-------------------------------------------------------------------\r\n\r\n");

	return;
}

void printLexemesToFile(struct LexemInfo* lexemInfoTable, char printBadLexeme, const char* filename) {
	FILE* file = fopen(filename, "wb");
	if (!file) {
		perror("Failed to open file");
		return;
	}

	if (printBadLexeme) {
		fprintf(file, "Bad lexeme:\r\n");
	}
	else {
		fprintf(file, "Lexemes table:\r\n");
	}
	fprintf(file, "-------------------------------------------------------------------\r\n");
	//fprintf(file, "index\t\tlexeme\t\tid\ttype\tifvalue\trow\tcol\r\n");
	fprintf(file, "index           lexeme          id      type    ifvalue row     col\r\n");
	fprintf(file, "-------------------------------------------------------------------\r\n");

	for (unsigned long long int index = 0; (!index || !printBadLexeme) && lexemInfoTable[index].lexemStr[0] != '\0'; ++index) {
		fprintf(file, "%5llu%17s%12llu%10llu%11llu%4lld%8lld\r\n",
			index,
			lexemInfoTable[index].lexemStr,
			lexemInfoTable[index].lexemId,
			lexemInfoTable[index].tokenType,
			lexemInfoTable[index].ifvalue,
			lexemInfoTable[index].row,
			lexemInfoTable[index].col);
	}
	fprintf(file, "-------------------------------------------------------------------\r\n\r\n");

	fclose(file);
}

// get identifier id
unsigned int getIdentifierId(char(*identifierIdsTable)[MAX_LEXEM_SIZE], char* str) {
	unsigned int index = 0;
	for (; identifierIdsTable[index][0] != '\0'; ++index) {
		if (!strncmp(identifierIdsTable[index], str, MAX_LEXEM_SIZE)) {
			return index;
		}
	}
	strncpy(identifierIdsTable[index], str, MAX_LEXEM_SIZE);
	identifierIdsTable[index + 1][0] = '\0'; // not necessarily for zero-init identifierIdsTable
	return index;
}

// try to get identifier
unsigned int tryToGetIdentifier(struct LexemInfo* lexemInfoInTable, char(*identifierIdsTable)[MAX_LEXEM_SIZE]) {
	char * identifiers_re = (char*) IDENTIFIERS_RE;
	//char identifiers_re[] = "_[A-Z][A-Z][A-Z][A-Z][A-Z][A-Z][A-Z]";

#ifdef USE_DFA_TO_ACCEPT_IDENTIFIER
	if (tryToAccept(&transitionTable3, transitionTable3FinitStates, lexemInfoInTable->lexemStr)) {
		lexemInfoInTable->lexemId = getIdentifierId(identifierIdsTable, lexemInfoInTable->lexemStr);
		lexemInfoInTable->tokenType = IDENTIFIER_LEXEME_TYPE;
		return SUCCESS_STATE;
	}
#else
	if (std::regex_match(std::string(lexemInfoInTable->lexemStr), std::regex(identifiers_re))) {
		lexemInfoInTable->lexemId = getIdentifierId(identifierIdsTable, lexemInfoInTable->lexemStr);
		lexemInfoInTable->tokenType = IDENTIFIER_LEXEME_TYPE;
		return SUCCESS_STATE;
	}
#endif

	return ~SUCCESS_STATE;
}

// try to get value
unsigned int tryToGetUnsignedValue(struct LexemInfo* lexemInfoInTable) {
	char * unsignedvalues_re = (char*) UNSIGNEDVALUES_RE;
	//char unsignedvalues_re[] = "0|[1-9][0-9]*";

#ifdef USE_DFA_TO_ACCEPT_UNSIGNEDVALUE
	if (tryToAccept(&transitionTable4, transitionTable4FinitStates, lexemInfoInTable->lexemStr)) {
		lexemInfoInTable->ifvalue = atoi(lastLexemInfoInTable->lexemStr);
		lexemInfoInTable->lexemId = MAX_VARIABLES_COUNT + MAX_KEYWORD_COUNT;
		lexemInfoInTable->tokenType = VALUE_LEXEME_TYPE;
		return SUCCESS_STATE;
	}
#else
	if (std::regex_match(std::string(lexemInfoInTable->lexemStr), std::regex(unsignedvalues_re))) {
		lexemInfoInTable->ifvalue = atoi(lastLexemInfoInTable->lexemStr);
		lexemInfoInTable->lexemId = MAX_VARIABLES_COUNT + MAX_KEYWORD_COUNT;
		lexemInfoInTable->tokenType = VALUE_LEXEME_TYPE;
		return SUCCESS_STATE;
	}
#endif
	return ~SUCCESS_STATE;
}

int commentRemover(char* text, const char* openStrSpc, const char* closeStrSpc) {
	bool eofAlternativeCloseStrSpcType = false;
	bool explicitCloseStrSpc = true;
	if (!strcmp(closeStrSpc, "\n")) {
		eofAlternativeCloseStrSpcType = true;
		explicitCloseStrSpc = false;
	}

	unsigned int commentSpace = 0;

	unsigned int textLength = strlen(text);               // strnlen(text, MAX_TEXT_SIZE);
	unsigned int openStrSpcLength = strlen(openStrSpc);   // strnlen(openStrSpc, MAX_TEXT_SIZE);
	unsigned int closeStrSpcLength = strlen(closeStrSpc); // strnlen(closeStrSpc, MAX_TEXT_SIZE);
	if (!closeStrSpcLength) {
		return -1; // no set closeStrSpc
	}
	unsigned char oneLevelComment = 0;
	if (!strncmp(openStrSpc, closeStrSpc, MAX_LEXEM_SIZE)) {
		oneLevelComment = 1;
	}

	for (unsigned int index = 0; index < textLength; ++index) {
		if (!strncmp(text + index, closeStrSpc, closeStrSpcLength) && (explicitCloseStrSpc || commentSpace)) {
			if (commentSpace == 1 && explicitCloseStrSpc) {
				for (unsigned int index2 = 0; index2 < closeStrSpcLength; ++index2) {
					text[index + index2] = ' ';
				}
			}
			else if (commentSpace == 1 && !explicitCloseStrSpc) {
				index += closeStrSpcLength - 1;
			}
			oneLevelComment ? commentSpace = !commentSpace : commentSpace = 0;
		}
		else if (!strncmp(text + index, openStrSpc, openStrSpcLength)) {
			oneLevelComment ? commentSpace = !commentSpace : commentSpace = 1;
		}

		if (commentSpace && text[index] != ' ' && text[index] != '\t' && text[index] != '\r' && text[index] != '\n') {
			text[index] = ' ';
		}

	}

	if (commentSpace && !eofAlternativeCloseStrSpcType) {
		return -1;
	}

	return 0;
}

void prepareKeyWordIdGetter(char* keywords_, char* keywords_re) {
	if (keywords_ == NULL || keywords_re == NULL) {
		return;
	}

	for (char* keywords_re_ = keywords_re, *keywords__ = keywords_; (*keywords_re_ != '\0') ? 1 : (*keywords__ = '\0', 0); (*keywords_re_ != '\\' || (keywords_re_[1] != '+' && keywords_re_[1] != '*' && keywords_re_[1] != '|')) ? *keywords__++ = *keywords_re_ : 0, ++keywords_re_);
}

unsigned int getKeyWordId(char* keywords_, char* lexemStr, unsigned int baseId) {
	if (keywords_ == NULL || lexemStr == NULL) {
		return ~0;
	}
	char* lexemInKeywords_ = keywords_;
	size_t lexemStrLen = strlen(lexemStr);
	if (!lexemStrLen) {
		return ~0;
	}

	for (; lexemInKeywords_ = strstr(lexemInKeywords_, lexemStr), lexemInKeywords_ != NULL && lexemInKeywords_[lexemStrLen] != '|' && lexemInKeywords_[lexemStrLen] != '\0'; ++lexemInKeywords_);

	return lexemInKeywords_ - keywords_ + baseId;
}

// try to get KeyWord
char tryToGetKeyWord(struct LexemInfo* lexemInfoInTable) {
	char * keywords_re = (char*) KEYWORDS_RE;
	//char keywords_re[] = ";|<<|>>|\\+|-|\\*|,|==|!=|:|\\(|\\)|NAME|DATA|BODY|END|EXIT|CONTINUE|GET|PUT|IF|ELSE|FOR|TO|DOWNTO|DO|WHILE|REPEAT|UNTIL|GOTO|DIV|MOD|<=|>=|NOT|AND|OR|INTEGER16";
	//char keywords_re[] = ";|<<|\\+\\+|--|\\*\\*|==|\\(|\\)|!=|:|name|data|body|end|get|put|for|to|downto|do|while|continue|exit|repeat|until|if|goto|div|mod|le|ge|not|and|or|long|int";
	char keywords_[sizeof(KEYWORDS_RE)] = { '\0' };
	prepareKeyWordIdGetter(keywords_, keywords_re);

#ifdef USE_DFA_TO_ACCEPT_KEYWORD
	if (tryToAccept(&transitionTable2, transitionTable2FinitStates, lexemInfoInTable->lexemStr)) {
		lexemInfoInTable->lexemId = getKeyWordId(keywords_, lexemInfoInTable->lexemStr, MAX_VARIABLES_COUNT);
		lexemInfoInTable->tokenType = KEYWORD_LEXEME_TYPE;
		return SUCCESS_STATE;
	}
#else
	if (std::regex_match(std::string(lexemInfoInTable->lexemStr), std::regex(keywords_re))) {
		lexemInfoInTable->lexemId = getKeyWordId(keywords_, lexemInfoInTable->lexemStr, MAX_VARIABLES_COUNT);
		lexemInfoInTable->tokenType = KEYWORD_LEXEME_TYPE;
		return SUCCESS_STATE;
	}
#endif
	return ~SUCCESS_STATE;
}

void setPositions(const char* text, struct LexemInfo* lexemInfoTable) {
	unsigned long long int line_number = 1;
	const char* pos = text, * line_start = text;

	if (lexemInfoTable) while (*pos != '\0' && lexemInfoTable->lexemStr[0] != '\0') {
		const char* line_end = strchr(pos, '\n');
		if (!line_end) {
			line_end = text + strlen(text);
		}

		char line_[4096], * line = line_; //!! TODO: ...
		strncpy(line, pos, line_end - pos);
		line[line_end - pos] = '\0';

		for (char* found_pos; lexemInfoTable->lexemStr[0] != '\0' && (found_pos = strstr(line, lexemInfoTable->lexemStr)); line += strlen(lexemInfoTable->lexemStr), ++lexemInfoTable) {
			lexemInfoTable->row = line_number;
			lexemInfoTable->col = found_pos - line_ + 1ull;
		}
		line_number++;
		pos = line_end;
		if (*pos == '\n') {
			pos++;
		}
	}
}

struct LexemInfo lexicalAnalyze(struct LexemInfo* lexemInfoInPtr, char(*identifierIdsTable)[MAX_LEXEM_SIZE]) {
	struct LexemInfo ifBadLexemeInfo; // = { 0 };

	if (tryToGetKeyWord(lexemInfoInPtr) == SUCCESS_STATE);
	else if (tryToGetIdentifier(lexemInfoInPtr, identifierIdsTable) == SUCCESS_STATE);
	else if (tryToGetUnsignedValue(lexemInfoInPtr) == SUCCESS_STATE);
	else {
		ifBadLexemeInfo.tokenType = UNEXPEXTED_LEXEME_TYPE;
	}

	return ifBadLexemeInfo;
}

struct LexemInfo tokenize(char* text, struct LexemInfo** lastLexemInfoInTable, char(*identifierIdsTable)[MAX_LEXEM_SIZE], struct LexemInfo(*lexicalAnalyzeFunctionPtr)(struct LexemInfo*, char(*)[MAX_LEXEM_SIZE])) {
	char * tokens_re = (char*) TOKENS_RE;
	//char tokens_re[] = ";|<<|>>|\\+|-|\\*|,|==|!=|:|\\(|\\)|<=|>=|[_0-9A-Za-z]+|[^ \t\r\f\v\n]";
	//char tokens_re[] = "<<|\\+\\+|--|\\*\\*|==|\\(|\\)|!=|[_0-9A-Za-z]+|[^ \t\r\f\v\n]";
#ifndef USE_DFA_FOR_TOKEN_PARSING
	std::regex tokens_re_(tokens_re);
#endif
	struct LexemInfo ifBadLexemeInfo; // = { 0 };
#ifndef USE_DFA_FOR_TOKEN_PARSING
	std::string stringText(text);
#endif

#ifdef USE_DFA_FOR_TOKEN_PARSING
	for (char* text_ = text, *text__ = text; *text_ != '\0'; ++ * lastLexemInfoInTable, text__ = ++text_){
		for (; *text_ != '\0' && !getFirstEntry(&transitionTable1, MAX_DFA_SCAN_LEXEM_SIZE_FOR_TOKEN_PARSING, transitionTable1FinitStates, &text_); ++text__, text_ = text__);
		if (*text_ == '\0') break;
		strncpy((*lastLexemInfoInTable)->lexemStr, text__, text_ - text__ + 1);
		(*lastLexemInfoInTable)->lexemStr[text_ - text__ + 1] = '\0';
		if ((ifBadLexemeInfo = (*lexicalAnalyzeFunctionPtr)(*lastLexemInfoInTable, identifierIdsTable)).tokenType == UNEXPEXTED_LEXEME_TYPE) {
			break;
		}
	}
#else
	for (std::sregex_token_iterator end, tokenIterator(stringText.begin(), stringText.end(), tokens_re_); tokenIterator != end; ++tokenIterator, ++ * lastLexemInfoInTable) {
		std::string str = *tokenIterator;
		strncpy((*lastLexemInfoInTable)->lexemStr, str.c_str(), MAX_LEXEM_SIZE);
		if ((ifBadLexemeInfo = (*lexicalAnalyzeFunctionPtr)(*lastLexemInfoInTable, identifierIdsTable)).tokenType == UNEXPEXTED_LEXEME_TYPE) {
			break;
		}
	}
#endif

	setPositions(text, lexemesInfoTable);

	if (ifBadLexemeInfo.tokenType == UNEXPEXTED_LEXEME_TYPE) {
		strncpy(ifBadLexemeInfo.lexemStr, (*lastLexemInfoInTable)->lexemStr, MAX_LEXEM_SIZE);
		ifBadLexemeInfo.row = (*lastLexemInfoInTable)->row;
		ifBadLexemeInfo.col = (*lastLexemInfoInTable)->col;
	}

	return ifBadLexemeInfo;
}

// after using this function use free(void *) function to release text buffer
size_t loadSource(char** text, char* fileName) {
	if (!fileName) {
		printf("No input file name\r\n");
		return 0;
	}

	FILE* file = fopen(fileName, "rb");

	if (file == NULL) {
		printf("File not loaded\r\n");
		return 0;
	}

	fseek(file, 0, SEEK_END);
	long fileSize_ = ftell(file);
	if (fileSize_ >= MAX_TEXT_SIZE) {
		printf("the file(%ld bytes) is larger than %d bytes\r\n", fileSize_, MAX_TEXT_SIZE);
		fclose(file);
		exit(2); // TODO: ...
		//return 0;
	}
	size_t fileSize = fileSize_;
	rewind(file);

	if (!text) {
		printf("Load source error\r\n");
		return 0;
	}
	*text = (char*)malloc(sizeof(char) * (fileSize + 1));
	if (*text == NULL) {
		fputs("Memory error", stderr);
		fclose(file);
		exit(2); // TODO: ...
		//return 0;
	}

	size_t result = fread(*text, sizeof(char), fileSize, file);
	if (result != fileSize) {
		fputs("Reading error", stderr);
		fclose(file);
		exit(3); // TODO: ...
		//return 0;
	}
	(*text)[fileSize] = '\0';

	fclose(file);

	return fileSize;
}

int main(int argc, char* argv[]) {
	char* text;
	char fileName[128] = DEFAULT_INPUT_FILE;
	char choice[2] = { fileName[0], fileName[1] };
	system("CLS");
	std::cout << "Enter file name(Enter \"" << choice[0] << "\" to use default \"" DEFAULT_INPUT_FILE "\"):";
	std::cin >> fileName;
	if (fileName[0] == choice[0] && fileName[1] == '\0') {
		fileName[1] = choice[1];
	}
	size_t sourceSize = loadSource(&text, fileName);
	if (!sourceSize) {
#ifdef RERUN_MODE
		(void)getchar();
		printf("\nEnter 'y' to rerun program action(to pass action enter other key): ");
		char valueByGetChar = getchar();
		if (valueByGetChar == 'y' || valueByGetChar == 'Y') {
			system((std::string("\"") + argv[0] + "\"").c_str());
		}
		return 0;
#else
		printf("Press Enter to exit . . .");
		(void)getchar();
		return 0;
#endif
	}
	printf("Original source:\r\n");
	printf("-------------------------------------------------------------------\r\n");
	printf("%s\r\n", text);
	printf("-------------------------------------------------------------------\r\n\r\n");
	int commentRemoverResult = commentRemover(text, "#*", "*#");
	if (commentRemoverResult) {
		printf("Comment remover return %d\r\n", commentRemoverResult);
		printf("Press Enter to exit . . .");
		(void)getchar();
		return 0;
	}
	printf("Source after comment removing:\r\n");
	printf("-------------------------------------------------------------------\r\n");
	printf("%s\r\n", text);
	printf("-------------------------------------------------------------------\r\n\r\n");

	struct LexemInfo ifBadLexemeInfo = tokenize(text, &lastLexemInfoInTable, identifierIdsTable, lexicalAnalyze);

	if (ifBadLexemeInfo.tokenType == UNEXPEXTED_LEXEME_TYPE) {
		UNEXPEXTED_LEXEME_TYPE;
		ifBadLexemeInfo.tokenType;
		printf("Lexical analysis detected unexpected lexeme\r\n");
		printLexemes(&ifBadLexemeInfo, 1);
	}
	else {
		printLexemes(lexemesInfoTable, 0);
	}

	free(text);

	(void)getchar();
#ifdef RERUN_MODE
	printf("\nEnter 'y' to rerun program action(to pass action enter other key): ");
	char valueByGetChar = getchar();
	if (valueByGetChar == 'y' || valueByGetChar == 'Y') {
		system((std::string("\"") + argv[0] + "\"").c_str());
	}
#endif

	return 0;
}