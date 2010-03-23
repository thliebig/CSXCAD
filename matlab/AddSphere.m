function CSX = AddSphere(CSX, propName, prio, center, rad)

sphere.ATTRIBUTE.Priority = prio;
sphere.ATTRIBUTE.Radius = rad;

sphere.Center.ATTRIBUTE.X=center(1);
sphere.Center.ATTRIBUTE.Y=center(2);
sphere.Center.ATTRIBUTE.Z=center(3);

CSX = Add2Property(CSX,propName, sphere, 'Sphere');