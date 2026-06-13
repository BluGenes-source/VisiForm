import React from 'react';
import {
  AbsoluteFill,
  Easing,
  interpolate,
  spring,
  useCurrentFrame,
  useVideoConfig,
} from 'remotion';

type BarDatum = {
  label: string;
  value: number;
  color: string;
};

const data: BarDatum[] = [
  {label: 'Design', value: 68, color: '#2F80ED'},
  {label: 'Build', value: 92, color: '#20B486'},
  {label: 'Test', value: 54, color: '#F2A93B'},
  {label: 'Review', value: 78, color: '#EB5757'},
  {label: 'Ship', value: 86, color: '#7B61FF'},
];

const maxValue = 100;
const chartHeight = 560;
const chartWidth = 1280;
const baselineY = 780;
const barWidth = 142;
const barGap = 112;
const firstBarX = (1920 - (data.length * barWidth + (data.length - 1) * barGap)) / 2;

const shadow = '0 28px 70px rgba(20, 25, 45, 0.18)';

export const FiveBarChart: React.FC = () => {
  const frame = useCurrentFrame();
  const {fps} = useVideoConfig();

  const titleProgress = spring({
    frame,
    fps,
    config: {damping: 180, stiffness: 90},
  });

  const gridProgress = interpolate(frame, [12, 42], [0, 1], {
    extrapolateLeft: 'clamp',
    extrapolateRight: 'clamp',
    easing: Easing.out(Easing.cubic),
  });

  const totalProgress = interpolate(frame, [115, 155], [0, 1], {
    extrapolateLeft: 'clamp',
    extrapolateRight: 'clamp',
    easing: Easing.out(Easing.cubic),
  });

  return (
    <AbsoluteFill
      style={{
        background:
          'linear-gradient(135deg, #F8FAFF 0%, #EEF6F1 52%, #FFF7EA 100%)',
        color: '#172033',
        fontFamily:
          'Inter, ui-sans-serif, system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif',
      }}
    >
      <div
        style={{
          position: 'absolute',
          inset: 0,
          background:
            'linear-gradient(90deg, rgba(47, 128, 237, 0.08) 0%, transparent 38%, rgba(32, 180, 134, 0.08) 100%)',
        }}
      />

      <div
        style={{
          position: 'absolute',
          left: 176,
          top: 106,
          transform: `translateY(${interpolate(titleProgress, [0, 1], [26, 0])}px)`,
          opacity: titleProgress,
        }}
      >
        <div
          style={{
            fontSize: 34,
            letterSpacing: 0,
            color: '#5A6478',
            fontWeight: 600,
            marginBottom: 18,
          }}
        >
          Workflow momentum
        </div>
        <div
          style={{
            fontSize: 86,
            lineHeight: 1,
            fontWeight: 800,
            letterSpacing: 0,
          }}
        >
          Five-bar progress chart
        </div>
      </div>

      <div
        style={{
          position: 'absolute',
          left: 120,
          top: 278,
          width: 1680,
          height: 654,
          background: 'rgba(255, 255, 255, 0.72)',
          border: '1px solid rgba(130, 145, 170, 0.24)',
          borderRadius: 8,
          boxShadow: shadow,
        }}
      />

      <svg
        width="1920"
        height="1080"
        viewBox="0 0 1920 1080"
        style={{position: 'absolute', inset: 0}}
      >
        {[0, 25, 50, 75, 100].map((tick) => {
          const y = baselineY - (tick / maxValue) * chartHeight;
          return (
            <g key={tick} opacity={gridProgress}>
              <line
                x1={(1920 - chartWidth) / 2}
                x2={(1920 + chartWidth) / 2}
                y1={y}
                y2={y}
                stroke="#B9C4D6"
                strokeWidth="2"
                strokeDasharray={tick === 0 ? undefined : '10 16'}
              />
              <text
                x={(1920 - chartWidth) / 2 - 36}
                y={y + 8}
                textAnchor="end"
                fill="#6B7588"
                fontSize="28"
                fontWeight="600"
              >
                {tick}
              </text>
            </g>
          );
        })}
      </svg>

      {data.map((item, index) => {
        const animatedHeight = spring({
          frame,
          fps,
          delay: 20 + index * 8,
          config: {damping: 190, stiffness: 96},
        });
        const height = (item.value / maxValue) * chartHeight * animatedHeight;
        const x = firstBarX + index * (barWidth + barGap);
        const y = baselineY - height;
        const counter = Math.round(
          interpolate(animatedHeight, [0, 1], [0, item.value], {
            extrapolateLeft: 'clamp',
            extrapolateRight: 'clamp',
          }),
        );

        return (
          <div key={item.label}>
            <div
              style={{
                position: 'absolute',
                left: x,
                top: y,
                width: barWidth,
                height,
                borderRadius: '8px 8px 0 0',
                background: `linear-gradient(180deg, ${item.color}, ${item.color}D9)`,
                boxShadow: `0 20px 42px ${item.color}55`,
              }}
            />
            <div
              style={{
                position: 'absolute',
                left: x - 34,
                top: y - 72,
                width: barWidth + 68,
                textAlign: 'center',
                color: item.color,
                fontSize: 42,
                fontWeight: 800,
                opacity: Math.min(1, animatedHeight * 1.2),
              }}
            >
              {counter}%
            </div>
            <div
              style={{
                position: 'absolute',
                left: x - 45,
                top: baselineY + 34,
                width: barWidth + 90,
                textAlign: 'center',
                color: '#273247',
                fontSize: 32,
                fontWeight: 700,
              }}
            >
              {item.label}
            </div>
          </div>
        );
      })}

      <div
        style={{
          position: 'absolute',
          right: 170,
          bottom: 98,
          width: 390,
          height: 78,
          borderRadius: 8,
          background: '#172033',
          color: 'white',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'center',
          fontSize: 31,
          fontWeight: 800,
          transform: `translateY(${interpolate(totalProgress, [0, 1], [34, 0])}px)`,
          opacity: totalProgress,
          boxShadow: '0 24px 56px rgba(23, 32, 51, 0.28)',
        }}
      >
        Average: {Math.round(data.reduce((sum, item) => sum + item.value, 0) / data.length)}%
      </div>
    </AbsoluteFill>
  );
};
