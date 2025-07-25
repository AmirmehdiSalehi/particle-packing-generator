/**
 * @file Interface.cpp
 * @brief Implementation of the Interface class
 * @author Amirmehdi Salehi
 * @date 2024
 * @copyright MIT License
 *
 * This file contains the implementation of the Interface class methods.
 */

#include "Interface.h"
#include <algorithm>

namespace Packing {

//=============================================================================
// Interface Implementation
//=============================================================================

Interface::Interface(int id) : id(id) {
    // Initialize bounding box to invalid values
    // Min values set to maximum, max values set to minimum
    bbox = {UINT32_MAX, UINT32_MAX, UINT32_MAX, 0, 0, 0};
}

Interface::~Interface() {
    // Automatic cleanup of STL containers
}

void Interface::addPoint(const Point3D& point) {
    // Define the 18-connected neighborhood (face and edge connections)
    // This includes the point itself and its 18 face/edge neighbors
    const int dx[] = {0, -1, 1, 0, 0, 0, 0, -1, -1, 1, 1, -1, -1, 1, 1, 0, 0, 0, 0};
    const int dy[] = {0, 0, 0, -1, 1, 0, 0, -1, 1, -1, 1, 0, 0, 0, 0, -1, -1, 1, 1};
    const int dz[] = {0, 0, 0, 0, 0, -1, 1, 0, 0, 0, 0, -1, 1, -1, 1, -1, 1, -1, 1};    
    
    // Add the point and its neighborhood to the segment
    for (int i = 0; i < 19; i++) {
        Point3D neighborPoint(point.x + dx[i], 
                            point.y + dy[i], 
                            point.z + dz[i]);
        segment.insert(neighborPoint);
    }

    // Update bounding box to include the point and its neighborhood
    std::array<uint32_t, 6> pointBbox = {
        static_cast<uint32_t>(point.x - 1), 
        static_cast<uint32_t>(point.y - 1), 
        static_cast<uint32_t>(point.z - 1), 
        static_cast<uint32_t>(point.x + 1), 
        static_cast<uint32_t>(point.y + 1), 
        static_cast<uint32_t>(point.z + 1)
    };
    mergeBbox(pointBbox);
}

void Interface::mergeBbox(std::array<uint32_t, 6>& otherBbox) {
    // Update minimum values (first three elements)
    for (int i = 0; i < 3; ++i) {
        bbox[i] = std::min(otherBbox[i], bbox[i]);
    }

    // Update maximum values (last three elements)
    for (int i = 3; i < 6; ++i) {
        bbox[i] = std::max(otherBbox[i], bbox[i]);
    }
}

bool Interface::withinBbox(const Point3D& point) const {
    // Check if point coordinates fall within the bounding box
    return (static_cast<uint32_t>(point.x) > bbox[0] && 
            static_cast<uint32_t>(point.y) > bbox[1] && 
            static_cast<uint32_t>(point.z) > bbox[2] &&
            static_cast<uint32_t>(point.x) < bbox[3] && 
            static_cast<uint32_t>(point.y) < bbox[4] && 
            static_cast<uint32_t>(point.z) < bbox[5]);
}

bool Interface::contains(const Point3D& point) const {
    // Check if the point exists in the segment set
    return segment.find(point) != segment.end();
}

bool Interface::mergeSegments(std::vector<std::shared_ptr<Interface>>& mergingSegments, 
                                    std::unordered_map<int, std::shared_ptr<Interface>>& interfacialSegments) {
    if (mergingSegments.empty()) {
        return false;
    }
    
    // Merge all provided segments into this one
    for (auto it = mergingSegments.begin(); it != mergingSegments.end(); ++it) {
        if ((*it)->getId() == id) {
            continue; // Skip self-merge
        }
        
        // Transfer all points from the other segment
        for (const auto& elem : (*it)->segment) {
            segment.insert(elem);
        }
        
        // Merge bounding boxes
        mergeBbox((*it)->bbox);
        
        // Remove the merged segment from the global map
        interfacialSegments.erase((*it)->getId());
    }
    
    return true;
}

} // namespace Packing