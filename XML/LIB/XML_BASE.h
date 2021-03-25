
typedef struct _XML_BASE
{
	UINT32	Key;
} XML_BASE;

#define MONSTER_XML_SIZE 1000
typedef struct _MONSTER_XML_DATA : XML_BASE
{
	UINT32 HpBase;
	UINT32 AttackBase;
	UINT32 MovementSpeed;
} MONSTER_XML_DATA;
