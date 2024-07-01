/*
SELECT
    allday.date AS 'date',
    allday.Energy AS Energy,
    dayt.energy AS EnrgDay,
    allday.Energy - dayt.energy AS EnrgNight,
    ROUND( dayt.energy/allday.Energy * 100 ) AS PctDay,
    allday.MaxP AS PMax,
    allday.AvgP AS Pavg,
    dayt.AvgP AS PavgDay,
    dayt.AvgpF AS PFday,
    allday.AvgpF AS PF
FROM
    ( SELECT
        DATE(dtime, 'localtime') AS 'date',
        ROUND( (cast(MAX(W) as real)- MIN(W))/1000,2 ) AS Energy,
        ROUND(cast (MAX(P) as real)/1000,2) AS `MaxP`,
        ROUND(AVG(P)) AS `AvgP`,
        ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
        FROM data
        WHERE devid='1'
            AND DATETIME(dtime, 'localtime') > DATE('now', '-1 MONTH', 'localtime')
        GROUP by DATE(dtime, 'localtime')
     ) allday
LEFT JOIN
    ( SELECT
        DATE(dtime, 'localtime') AS 'date',
        ROUND( (cast(MAX(W) as real)- MIN(W))/1000,2 ) AS Energy,
        ROUND(AVG(P)) AS `AvgP`,
        ROUND(100*AVG(p/(U*I)),1) as `AvgpF`
      FROM data
        WHERE devid='1'
            AND DATETIME(dtime, 'localtime') > DATE('now', '-1 MONTH')
            AND ( cast(strftime('%H', dtime, 'localtime') as int) > 7 OR cast( strftime('%H', dtime, 'localtime') as int) < 22)
        GROUP by DATE(dtime, 'localtime')
    ) dayt
ON allday.date = dayt.date
ORDER by date DESC;
*/

-- Select monthly stat (last 12 month)
-- kW
/*
SELECT
    allday.Month AS 'Month',
    allday.Energy AS Energy,
    dayt.energy AS EnrgDay,
    allday.Energy - dayt.energy AS EnrgNight,
    ROUND( dayt.energy/allday.Energy * 100 ) AS PctDay,
    allday.AEpD AS EApD,
    allday.MaxP AS PMax,
    allday.AvgP AS Pavg,
    dayt.AvgP   AS PavgDay,
    dayt.AvgpF  AS PFday,
    allday.AvgpF AS PF
FROM
    ( SELECT
        strftime('%m', dtime, 'localtime') AS 'Month',
        ROUND( (cast (MAX(W) as real) - MIN(W))/1000, 2 ) AS Energy,
        ROUND((cast (MAX(W) as real) - MIN(W))/ (MAX(strftime('%d', dtime, 'localtime'))-MIN(strftime('%d', dtime, 'localtime'))+1) /1000, 2 ) AS AEpD,
        ROUND(cast(MAX(P)as real)/1000,2) AS `MaxP`,
        ROUND(AVG(P)/1000,2) AS `AvgP`,
        ROUND(100*AVG(p/(U*I))) as `AvgpF`
        FROM data
        WHERE devid='1' AND DATETIME(dtime, 'localtime') > DATE('now', 'start of month', '-1 YEAR', 'localtime')
        GROUP by strftime('%m', dtime, 'localtime')
        ORDER by dtime DESC
     ) allday
LEFT JOIN
    ( SELECT
      perday.Month,
      SUM(perday.Energy) AS Energy,
      ROUND(AVG(perday.P)/1000,2) AS `AvgP`,
      ROUND(100*AVG(perday.AvgpF)) as `AvgpF`
      FROM (
        SELECT
            strftime('%m', dtime, 'localtime') AS 'Month',
            ROUND( (cast (MAX(W) as real) - MIN(W))/1000,2 ) AS Energy,
            AVG(P) AS `P`,
            AVG(p/(U*I)) as `AvgpF`
        FROM data
            WHERE devid='1'
		AND cast(strftime('%H', dtime, 'localtime') as int) >= 7 AND cast( strftime('%H', dtime, 'localtime') as int) < 23
		AND DATETIME(dtime, 'localtime') > DATE('now', 'start of month', '-1 YEAR', 'localtime')
            GROUP by DATE(dtime)
        ) perday
     GROUP by perday.Month
    ) dayt
ON allday.Month = dayt.Month;
-- ORDER by Month DESC;
*/


-- Select last recorded values
SELECT
    DATETIME(dtime, 'localtime') AS dtime, U, I, P, W, ROUND(100*P/(U*I)) as PF
FROM data
WHERE devid='1'
ORDER BY dtime DESC
LIMIT 1;
