// body is a class that encaps
#ifndef BODY_H
#define BODY_H
class Body {
  public:
    float posX;
    float posY;
    float velX;
    float velY;
    float mass;
    float radius;
    Body() : posX(0), posY(0), velX(0), velY(0), mass(0), radius(0) {}
    Body(float x, float y, float vx, float vy, float m, float r) 
      : posX(x), posY(y), velX(vx), velY(vy), mass(m), radius(r) {}
    
};
#endif // BODY_H