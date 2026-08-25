#pragma once

#include "../Board.h"
#include "GreedyRobot.h"

namespace dab::__detail__::robot {

class ImproveGreedyRobot : public GreedyRobot {
 public:
  ImproveGreedyRobot() = default;

  template <typename Board>
  iterable::Span<const model::Edge> BestCandidateEdges(const Board& board);
  template <typename Board>
  model::Edge SearchOne(const Board& board);

 private:
  template <typename Board>
  bool HasSafeMove(const Board& board) const;
  template <typename Board>
  model::Edge FindSingleBoxEdge(const Board& board) const;
  template <typename Board>
  Int TakableCount(const Board& board);
  template <typename Board>
  model::Edge OpenShortestChain(const Board& board);

  Int CandidateEdgesSize = 0;
  board::ScoreableCountBoard SimulationBoardBackup;
  board::ScoreableCountBoard SimulationBoard;
};

}  // namespace dab::__detail__::robot