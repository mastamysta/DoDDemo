# Object Oriented vs Data Oriented Design for Cache Efficiency

## Premise

We have a N bodies with random (double precision) mass floating in 2-dimensional space at random (discrete) positions. We need to calulate accellerations, velocities and new positions for each of the bodies over a thousand 1-second epochs, 
taking into account the effect of gravity between the objects. It is not considered acceptable to use an approximation method such as [Barnes-Hut] (https://en.wikipedia.org/wiki/Barnes%E2%80%93Hut_simulation), you must instead perform
an N^2 iteration for each epoch.

## Object Oriented Approach

I think that the intuitive object oriented design for this system is to create a class for a body which stores mass & velocity of the object.
