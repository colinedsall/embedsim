#ifndef GRAPHICS_OBJECTS_HPP
#define GRAPHICS_OBJECTS_HPP

#include <QPainter>
#include <QColor>
#include <QString>
#include <memory>
#include <vector>

// Enum for fill styles
enum class FillStyle {
    Solid,
    Hollow
};

// Base class for all graphics objects
class GraphicsObject
{
public:
    GraphicsObject(int x, int y, const QColor& color, int id);
    virtual ~GraphicsObject() = default;
    
    // Pure virtual method for drawing
    virtual void draw(QPainter& painter) = 0;
    
    // Getters
    int getX() const { return x; }
    int getY() const { return y; }
    QColor getColor() const { return color; }
    int getId() const { return id; }
    QString getType() const { return type; }
    
    // Setters
    void setColor(const QColor& newColor) { color = newColor; }
    void setPosition(int newX, int newY) { x = newX; y = newY; }
    
protected:
    int x, y;
    QColor color;
    int id;
    QString type;
};

// Line object
class LineObject : public GraphicsObject
{
public:
    LineObject(int x1, int y1, int x2, int y2, const QColor& color, int id);
    void draw(QPainter& painter) override;
    
    int getX2() const { return x2; }
    int getY2() const { return y2; }
    
private:
    int x2, y2;
};

// Rectangle object
class RectangleObject : public GraphicsObject
{
public:
    RectangleObject(int x, int y, int width, int height, const QColor& color, int id, FillStyle fillStyle = FillStyle::Solid);
    void draw(QPainter& painter) override;
    
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    FillStyle getFillStyle() const { return fillStyle; }
    void setFillStyle(FillStyle newFillStyle) { fillStyle = newFillStyle; }
    
private:
    int width, height;
    FillStyle fillStyle;
};

// Circle object
class CircleObject : public GraphicsObject
{
public:
    CircleObject(int x, int y, int radius, const QColor& color, int id, FillStyle fillStyle = FillStyle::Solid);
    void draw(QPainter& painter) override;
    
    int getRadius() const { return radius; }
    FillStyle getFillStyle() const { return fillStyle; }
    void setFillStyle(FillStyle newFillStyle) { fillStyle = newFillStyle; }
    
private:
    int radius;
    FillStyle fillStyle;
};

// Graphics manager for handling all objects
class GraphicsManager
{
public:
    GraphicsManager();
    ~GraphicsManager();
    
    // Object creation methods
    int createLine(int x1, int y1, int x2, int y2, const QColor& color);
    int createRectangle(int x, int y, int width, int height, const QColor& color, FillStyle fillStyle = FillStyle::Solid);
    int createCircle(int x, int y, int radius, const QColor& color, FillStyle fillStyle = FillStyle::Solid);
    
    // Object management
    bool removeObject(int id);
    void clearAll();
    void setObjectColor(int id, const QColor& color);
    void setObjectPosition(int id, int x, int y);
    void setObjectFillStyle(int id, FillStyle fillStyle);
    
    // Drawing
    void drawAll(QPainter& painter);
    
    // Information
    int getObjectCount() const { return objects.size(); }
    QString getObjectInfo(int id) const;
    QString getAllObjectsInfo() const;
    
    // Memory management
    size_t getMemoryUsage() const;
    
private:
    std::vector<std::unique_ptr<GraphicsObject>> objects;
    int nextId;
    
    GraphicsObject* findObject(int id);
    const GraphicsObject* findObject(int id) const;
};

#endif // GRAPHICS_OBJECTS_HPP
