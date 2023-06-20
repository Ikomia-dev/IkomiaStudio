--Add scientific article URL
ALTER TABLE process ADD articleUrl TEXT;
--Rename ikomiaVersion to ikomiaMinVersion
ALTER TABLE process RENAME COLUMN ikomiaVersion TO minIkomiaVersion;
--Add Ikomia max compatible version
ALTER TABLE process ADD maxIkomiaVersion TEXT;
--Add compatible Python minimum version
ALTER TABLE process ADD minPythonVersion TEXT;
--Add compatible Python maximum version
ALTER TABLE process ADD maxPythonVersion TEXT;
--Add original implementation repository
ALTER TABLE process ADD originalRepository TEXT;
--Add algorithme type (inference, train, dataset...)
ALTER TABLE process ADD algoType INTEGER;
--Add algorithme tasks
ALTER TABLE process ADD algoTasks TEXT;
--Remove algorithm serverId
ALTER TABLE process DROP serverId;
--Remove algorithm userId
ALTER TABLE process DROP userId;
