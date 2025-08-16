#include "graphics_objects.hpp"
#include <QPainter>
#include <QDebug>
#include <sstream>

// Base GraphicsObject implementation
GraphicsObject::GraphicsObject(int x, int y, const QColor& color, int id)
    : x(x), y(y), color(color), id(id), type("Unknown")
{
}

// LineObject implementation
LineObject::LineObject(int x1, int y1, int x2, int y2, const QColor& color, int id)
    : GraphicsObject(x1, y1, color, id), x2(x2), y2(y2)
{
    type = "Line";
}

void LineObject::draw(QPainter& painter)
{
    painter.setPen(QPen(color, 2, Qt::SolidLine));
    painter.drawLine(x, y, x2, y2);
}

// RectangleObject implementation
RectangleObject::RectangleObject(int x, int y, int width, int height, const QColor& color, int id, FillStyle fillStyle)
    : GraphicsObject(x, y, color, id), width(width), height(height), fillStyle(fillStyle)
{
    type = "Rectangle";
}

void RectangleObject::draw(QPainter& painter)
{
    painter.setPen(QPen(color, 2, Qt::SolidLine));
    
    if (fillStyle == FillStyle::Solid) {
        painter.setBrush(QBrush(color));
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    painter.drawRect(x, y, width, height);
}

// CircleObject implementation
CircleObject::CircleObject(int x, int y, int radius, const QColor& color, int id, FillStyle fillStyle)
    : GraphicsObject(x, y, color, id), radius(radius), fillStyle(fillStyle)
{
    type = "Circle";
}

void CircleObject::draw(QPainter& painter)
{
    painter.setPen(QPen(color, 2, Qt::SolidLine));
    
    if (fillStyle == FillStyle::Solid) {
        painter.setBrush(QBrush(color));
    } else {
        painter.setBrush(Qt::NoBrush);
    }
    
    painter.drawEllipse(x - radius, y - radius, radius * 2, radius * 2);
}

// GraphicsManager implementation
GraphicsManager::GraphicsManager() : nextId(1)
{
}

GraphicsManager::~GraphicsManager()
{
    clearAll();
}

int GraphicsManager::createLine(int x1, int y1, int x2, int y2, const QColor& color)
{
    auto line = std::make_unique<LineObject>(x1, y1, x2, y2, color, nextId);
    int id = nextId++;
    objects.push_back(std::move(line));
    return id;
}

int GraphicsManager::createRectangle(int x, int y, int width, int height, const QColor& color, FillStyle fillStyle)
{
    auto rect = std::make_unique<RectangleObject>(x, y, width, height, color, nextId, fillStyle);
    int id = nextId++;
    objects.push_back(std::move(rect));
    return id;
}

int GraphicsManager::createCircle(int x, int y, int radius, const QColor& color, FillStyle fillStyle)
{
    auto circle = std::make_unique<CircleObject>(x, y, radius, color, nextId, fillStyle);
    int id = nextId++;
    objects.push_back(std::move(circle));
    return id;
}

bool GraphicsManager::removeObject(int id)
{
    auto it = std::find_if(objects.begin(), objects.end(),
        [id](const std::unique_ptr<GraphicsObject>& obj) {
            return obj->getId() == id;
        });
    
    if (it != objects.end()) {
        objects.erase(it);
        return true;
    }
    return false;
}

void GraphicsManager::clearAll()
{
    objects.clear();
}

void GraphicsManager::setObjectColor(int id, const QColor& color)
{
    if (GraphicsObject* obj = findObject(id)) {
        obj->setColor(color);
    }
}

void GraphicsManager::setObjectPosition(int id, int x, int y)
{
    if (GraphicsObject* obj = findObject(id)) {
        obj->setPosition(x, y);
    }
}

void GraphicsManager::setObjectFillStyle(int id, FillStyle fillStyle)
{
    if (RectangleObject* rect = dynamic_cast<RectangleObject*>(findObject(id))) {
        rect->setFillStyle(fillStyle);
    } else if (CircleObject* circle = dynamic_cast<CircleObject*>(findObject(id))) {
        circle->setFillStyle(fillStyle);
    }
}

void GraphicsManager::drawAll(QPainter& painter)
{
    // Draw all objects in order (later objects appear on top)
    for (const auto& obj : objects) {
        obj->draw(painter);
    }
}

QString GraphicsManager::getObjectInfo(int id) const
{
    if (const GraphicsObject* obj = findObject(id)) {
        QString info = QString("ID: %1, Type: %2, Pos: (%3,%4), Color: #%5")
            .arg(obj->getId())
            .arg(obj->getType())
            .arg(obj->getX())
            .arg(obj->getY())
            .arg(obj->getColor().rgb(), 0, 16);
        
        // Add type-specific info
        if (obj->getType() == "Line") {
            const LineObject* line = static_cast<const LineObject*>(obj);
            info += QString(", End: (%1,%2)").arg(line->getX2()).arg(line->getY2());
        } else if (obj->getType() == "Rectangle") {
            const RectangleObject* rect = static_cast<const RectangleObject*>(obj);
            info += QString(", Size: %1x%2, Fill: %3")
                .arg(rect->getWidth())
                .arg(rect->getHeight())
                .arg(rect->getFillStyle() == FillStyle::Solid ? "Solid" : "Hollow");
        } else if (obj->getType() == "Circle") {
            const CircleObject* circle = static_cast<const CircleObject*>(obj);
            info += QString(", Radius: %1, Fill: %2")
                .arg(circle->getRadius())
                .arg(circle->getFillStyle() == FillStyle::Solid ? "Solid" : "Hollow");
        }
        
        return info;
    }
    return QString("Object ID %1 not found").arg(id);
}

QString GraphicsManager::getAllObjectsInfo() const
{
    if (objects.empty()) {
        return "No graphics objects";
    }
    
    QString info = QString("Graphics Objects (%1 total):\n").arg(objects.size());
    for (const auto& obj : objects) {
        info += getObjectInfo(obj->getId()) + "\n";
    }
    return info;
}

size_t GraphicsManager::getMemoryUsage() const
{
    size_t total = 0;
    
    // Base memory for objects vector
    total += sizeof(std::vector<std::unique_ptr<GraphicsObject>>);
    
    // Memory for each object
    for (const auto& obj : objects) {
        total += sizeof(GraphicsObject);
        
        // Add size for derived classes
        if (obj->getType() == "Line") {
            total += sizeof(LineObject) - sizeof(GraphicsObject);
        } else if (obj->getType() == "Rectangle") {
            total += sizeof(RectangleObject) - sizeof(GraphicsObject);
        } else if (obj->getType() == "Circle") {
            total += sizeof(CircleObject) - sizeof(GraphicsObject);
        }
    }
    
    return total;
}

GraphicsObject* GraphicsManager::findObject(int id)
{
    auto it = std::find_if(objects.begin(), objects.end(),
        [id](const std::unique_ptr<GraphicsObject>& obj) {
            return obj->getId() == id;
        });
    
    return (it != objects.end()) ? it->get() : nullptr;
}

const GraphicsObject* GraphicsManager::findObject(int id) const
{
    auto it = std::find_if(objects.begin(), objects.end(),
        [id](const std::unique_ptr<GraphicsObject>& obj) {
            return obj->getId() == id;
        });
    
    return (it != objects.end()) ? it->get() : nullptr;
}
