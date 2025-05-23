#ifndef FIRE2_H
#define FIRE2_H

// Forward declare LSD structure if its definition is not included here
struct LSD; 

namespace FireEffect2 {

    // Initializes the fire effect (palette, buffer)
    // Takes the screen dimensions and a pointer to the LSD graphics context.
    bool initFire2(LSD* lsdContext, int screenWidth, int screenHeight);

    // Updates the fire simulation by one step
    void updateFire2();

    // Renders the current state of the fire to the screen
    // Takes a pointer to the LSD graphics context.
    void renderFire2(LSD* lsdContext);

    // De-initializes the fire effect (frees memory)
    void closeFire2();

} // namespace FireEffect2

#endif // FIRE2_H
