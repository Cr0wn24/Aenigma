#ifndef ENTITY_H
#define ENTITY_H

typedef enum EntityType {
    EntityType_Null,
    EntityType_Water,
    EntityType_Grass,
    EntityType_Wall,
    EntityType_Tim,
    EntityType_MovableWall,
    EntityType_DoorPortalHori,
    EntityType_DoorPortalVert,
    EntityType_Emitter,
    EntityType_Door,
    EntityType_MossyWall,
} EntityType;

typedef enum FacingDirection {
    Direction_Still,

    Direction_Up,
    Direction_Down,
    Direction_Left,
    Direction_Right,
}  FacingDirection;

typedef struct EntityGroup {
    u32 count;
    Entity *entities[4];
} EntityGroup;

typedef struct EntityMoveEvent {
    b32 active;
    TileMapPosition target_pos;
    TileMapPosition start_pos;
    f32 timer;
    f32 duration;
} EntityMoveEvent;

// used for serializing
typedef struct EntityPointer {
    TileMapPosition pos;
    EntityType type;
} EntityPointer;

typedef struct Entity {
    EntityType type;
    u32 id;
    b32 active;
    TileMapPosition pos;
    FacingDirection facing_direction;
    EntityMoveEvent move_event;
    b32 initialized;

    // Door
    b32 open;

    // Emitter
    u32 num_particles;
    Particle *particles;
} Entity;

#endif