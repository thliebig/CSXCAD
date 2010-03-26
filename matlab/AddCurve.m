function CSX = AddCurve(CSX, propName, prio, points)

curve.ATTRIBUTE.Priority = prio;

curve.Vertex={};
for s=1:size(points,2)
    curve.Vertex{end+1}.ATTRIBUTE.X = points(1,s);
    curve.Vertex{end}.ATTRIBUTE.Y = points(2,s);
    curve.Vertex{end}.ATTRIBUTE.Z = points(3,s);
end

CSX = Add2Property(CSX,propName, curve, 'Curve');